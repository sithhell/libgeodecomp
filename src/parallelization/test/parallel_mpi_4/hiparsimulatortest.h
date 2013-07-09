#include <boost/shared_ptr.hpp>
#include <cxxtest/TestSuite.h>
#include <sstream>
#include <libgeodecomp/io/mocksteerer.h>
#include <libgeodecomp/io/mockwriter.h>
#include <libgeodecomp/io/teststeerer.h>
#include <libgeodecomp/io/parallelmemorywriter.h>
#include <libgeodecomp/io/paralleltestwriter.h>
#include <libgeodecomp/io/testinitializer.h>
#include <libgeodecomp/loadbalancer/mockbalancer.h>
#include <libgeodecomp/misc/testcell.h>
#include <libgeodecomp/misc/testhelper.h>
#include <libgeodecomp/parallelization/hiparsimulator.h>
#include <libgeodecomp/parallelization/hiparsimulator/partitions/zcurvepartition.h>

using namespace LibGeoDecomp;
using namespace HiParSimulator;

namespace LibGeoDecomp {
namespace HiParSimulator {

class HiParSimulatorTest : public CxxTest::TestSuite
{
public:
    typedef GridBase<TestCell<2>, 2> GridBaseType;
    typedef HiParSimulator<TestCell<2>, ZCurvePartition<2> > SimulatorType;
    typedef ParallelMemoryWriter<TestCell<2> > MemoryWriterType;
    typedef MockSteerer<TestCell<2> > MockSteererType;
    typedef TestSteerer<2 > TestSteererType;

    void setUp()
    {
        int width = 131;
        int height = 241;
        dim = Coord<2>(width, height);
        maxSteps = 101;
        firstStep = 20;
        firstCycle = firstStep * TestCell<2>::nanoSteps();
        TestInitializer<TestCell<2> > *init = new TestInitializer<TestCell<2> >(
            dim, maxSteps, firstStep);

        outputPeriod = 4;
        loadBalancingPeriod = 31;
        ghostZoneWidth = 10;
        sim.reset(new SimulatorType(
                    init,
                    new MockBalancer(),
                    loadBalancingPeriod,
                    ghostZoneWidth));
        mockWriter = new MockWriter();
        memoryWriter = new MemoryWriterType(outputPeriod);
        sim->addWriter(mockWriter);
        sim->addWriter(memoryWriter);
    }

    void tearDown()
    {
        sim.reset();
    }

    void testStep()
    {
        sim->step();
        TS_ASSERT_EQUALS((31 - 1)       * 27, sim->timeToNextEvent());
        TS_ASSERT_EQUALS((101 - 20 - 1) * 27, sim->timeToLastEvent());
        sim->step();
        sim->step();
        sim->step();

        TS_ASSERT_EQUALS((31 - 4)       * 27, sim->timeToNextEvent());
        TS_ASSERT_EQUALS((101 - 20 - 4) * 27, sim->timeToLastEvent());

        std::stringstream expectedEvents;
        expectedEvents << "initialized()\ninitialized()\n";
        for (int t = 21; t < 25; t += 1) {
            expectedEvents << "stepFinished(step=" << t << ")\n"
                           << "stepFinished(step=" << t << ")\n";
        }
        TS_ASSERT_EQUALS(expectedEvents.str(), mockWriter->events());

        for (int t = 20; t < 25; t += outputPeriod) {
            int globalNanoStep = t * TestCell<2>::nanoSteps();
            MemoryWriterType::GridMap grids = memoryWriter->getGrids();
            TS_ASSERT_TEST_GRID(
                MemoryWriterType::GridType,
                grids[t],
                globalNanoStep);
            TS_ASSERT_EQUALS(dim, grids[t].getDimensions());
        }
    }

    void testRun()
    {
        sim->run();

        for (unsigned t = firstStep; t < maxSteps; t += outputPeriod) {
            unsigned globalNanoStep = t * TestCell<2>::nanoSteps();
            MemoryWriterType::GridMap grids = memoryWriter->getGrids();
            TS_ASSERT_TEST_GRID(
                MemoryWriterType::GridType,
                grids[t],
                globalNanoStep);
            TS_ASSERT_EQUALS(dim, grids[t].getDimensions());
        }

        // check last step, too
        unsigned t = maxSteps;
        unsigned globalNanoStep = t * TestCell<2>::nanoSteps();
        MemoryWriterType::GridMap grids = memoryWriter->getGrids();
        TS_ASSERT_TEST_GRID(
            MemoryWriterType::GridType,
            grids[t],
            globalNanoStep);
        TS_ASSERT_EQUALS(dim, grids[t].getDimensions());

        if (MPILayer().rank() == 0) {
            std::string expectedEvents;
            for (int i = 0; i < 2; ++i) {
                expectedEvents += "balance() [7892, 7893, 7893, 7893] [1, 1, 1, 1]\n";
            }

            TS_ASSERT_EQUALS(expectedEvents, MockBalancer::events);
        }
    }

    void testSteererCallback()
    {
        std::stringstream events;
        sim->addSteerer(new MockSteererType(5, &events));
        sim->run();
        sim.reset();

        std::stringstream expected;
        expected << "created, period = 5\n";
        for (int i = 25; i <= 101; i += 5) {
            expected << "nextStep(" << i << ")\n";
            expected << "nextStep(" << i << ")\n";
        }
        expected << "deleted\n";

        TS_ASSERT_EQUALS(events.str(), expected.str());
    }

    void testSteererFunctionality()
    {
        sim->addSteerer(new TestSteererType(5, 25, 4711 * 27));
        sim->run();

        const Region<2> *region = &sim->updateGroup->partitionManager->innerSet(ghostZoneWidth);
        const GridBaseType *grid = &sim->updateGroup->grid();
        int cycle = 101 * 27 + 4711 * 27;

        TS_ASSERT_TEST_GRID_REGION(
            GridBaseType,
            *grid,
            *region,
            cycle);
    }

    void testParallelWriterInvocation()
    {
        unsigned period = 4;
        SuperVector<unsigned> expectedSteps;
        SuperVector<WriterEvent> expectedEvents;
        expectedSteps << 20
                      << 24
                      << 28
                      << 32
                      << 36
                      << 40
                      << 44
                      << 48
                      << 52
                      << 56
                      << 60
                      << 64
                      << 68
                      << 72
                      << 76
                      << 80
                      << 84
                      << 88
                      << 92
                      << 96
                      << 100
                      << 101;
        expectedEvents << WRITER_INITIALIZED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_STEP_FINISHED
                       << WRITER_ALL_DONE;

        sim->addWriter(new ParallelTestWriter(period, expectedSteps, expectedEvents));
        sim->run();
    }

private:
    boost::shared_ptr<SimulatorType> sim;
    Coord<2> dim;
    unsigned maxSteps;
    unsigned firstStep;
    unsigned firstCycle;
    unsigned outputPeriod;
    unsigned loadBalancingPeriod;
    unsigned ghostZoneWidth;
    MockWriter *mockWriter;
    MemoryWriterType *memoryWriter;
};

}
}
