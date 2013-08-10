#include <sstream>
#include <vector>
#include <cxxtest/TestSuite.h>
#include <libgeodecomp/misc/cellapitraits.h>
#include <libgeodecomp/misc/grid.h>
#include <libgeodecomp/misc/testhelper.h>
#include <libgeodecomp/misc/updatefunctor.h>
#include <libgeodecomp/misc/updatefunctortestbase.h>

using namespace LibGeoDecomp;

std::stringstream myLog;

class BasicCell
{
public:
    typedef Stencils::Moore<2, 1> Stencil;
    typedef Topologies::Torus<2>::Topology Topology;

    class API : public CellAPITraits::Base
    {};

    static int nanoSteps()
    {
        return 1;
    }

    template<typename NEIGHBORHOOD>
    void update(const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "BasicCell::update(nanoStep = " << nanoStep << ")\n";
    }
};

class LineUpdateCell
{
public:
    typedef Stencils::Moore<2, 1> Stencil;
    typedef Topologies::Torus<2>::Topology Topology;

    class API : public CellAPITraits::Line
    {};

    static int nanoSteps()
    {
        return 1;
    }

    template<typename NEIGHBORHOOD>
    void update(const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "LineUpdateCell::update(nanoStep = " << nanoStep << ")\n";
    }

    // won't ever be called as all current update functors support
    // updateLine only with fixed neighborhoods
    template<typename NEIGHBORHOOD>
    static void updateLine(LineUpdateCell *target, long *x, long endX, const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "LineUpdateCell::updateLine(x = " << *x << ", endX = " << endX << ", nanoStep = " << nanoStep << ")\n";

    }
};

class FixedCell
{
public:
    typedef Stencils::Moore<2, 1> Stencil;
    typedef Topologies::Torus<2>::Topology Topology;

    class API : public CellAPITraits::Fixed
    {};

    static int nanoSteps()
    {
        return 1;
    }

    template<typename NEIGHBORHOOD>
    void update(const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "FixedCell::update(nanoStep = " << nanoStep << ")\n";
    }

};

class FixedLineUpdateCell
{
public:
    typedef Stencils::Moore<2, 1> Stencil;
    typedef Topologies::Torus<2>::Topology Topology;

    class API : public CellAPITraits::Fixed, public CellAPITraits::Line
    {};

    static int nanoSteps()
    {
        return 1;
    }

    template<typename NEIGHBORHOOD>
    void update(const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "FixedLineUpdateCell::update(nanoStep = " << nanoStep << ")\n";
    }

    template<typename NEIGHBORHOOD>
    static void updateLine(FixedLineUpdateCell *target, long *x, long endX, const NEIGHBORHOOD& hood, int nanoStep)
    {
        myLog << "FixedLineUpdateCell::updateLine(x = " << *x << ", endX = " << endX << ", nanoStep = " << nanoStep << ")\n";

    }
};

class MySoATestCell
{
public:
    typedef Stencils::Moore<3, 1> Stencil;
    typedef Topologies::Torus<3>::Topology Topology;

    class API : public CellAPITraits::Fixed,
                public CellAPITraitsFixme::SupportsSoA,
                public CellAPITraitsFixme::SupportsUpdateLineX
    {};

    static int nanoSteps()
    {
        return 1;
    }

    MySoATestCell(
        double temp = 0.0,
        bool alive = false) :
        temp(temp),
        alive(alive)
    {}

    template<typename NEIGHBORHOOD>
    void update(const NEIGHBORHOOD& hood, int nanoStep)
    {
    }

    template<typename ACCESSOR1, typename ACCESSOR2>
    static void updateLineX(
        const Streak<3>& streak,
        const Coord<3>& targetOrigin,
        const unsigned nanoStep,
        ACCESSOR1 hoodOld, int *indexOld, ACCESSOR2 hoodNew, int *indexNew)
    {
        std::cout << "cowabunga!\n";
    }

    double temp;
    bool alive;
};

LIBFLATARRAY_REGISTER_SOA(MySoATestCell, ((double)(temp))((bool)(alive)))

namespace LibGeoDecomp {

class UpdateFunctorTest : public CxxTest::TestSuite
{
public:
    template<class STENCIL>
    class UpdateFunctorTestHelper : public UpdateFunctorTestBase<STENCIL>
    {
    public:
        using UpdateFunctorTestBase<STENCIL>::DIM;
        typedef typename UpdateFunctorTestBase<STENCIL>::TestCellType TestCellType;
        typedef typename UpdateFunctorTestBase<STENCIL>::GridType GridType;
        typedef STENCIL Stencil;

        virtual void callFunctor(
            const Streak<DIM>& streak,
            const GridType& gridOld,
            GridType *gridNew,
            unsigned nanoStep)
        {
            UpdateFunctor<TestCellType>()(
                streak, streak.origin, gridOld, gridNew, nanoStep);
        }
    };

    void testSelector()
    {
        checkSelector<BasicCell>(
            "BasicCell::update(nanoStep = 0)\n", 8);
        checkSelector<LineUpdateCell>(
            "LineUpdateCell::update(nanoStep = 0)\n", 8);
        checkSelector<FixedCell>(
            "FixedCell::update(nanoStep = 0)\n", 8);
        checkSelector<FixedLineUpdateCell>(
            "FixedLineUpdateCell::update(nanoStep = 0)\nFixedLineUpdateCell::updateLine(x = 0, endX = 7, nanoStep = 0)\nFixedLineUpdateCell::update(nanoStep = 0)\n", 1);
    }

    void testMoore2D()
    {
        UpdateFunctorTestHelper<Stencils::Moore<2, 1> >().testSimple(3);
        UpdateFunctorTestHelper<Stencils::Moore<2, 1> >().testSplittedTraversal(3);
    }

    void testMoore3D()
    {
        UpdateFunctorTestHelper<Stencils::Moore<3, 1> >().testSimple(3);
        UpdateFunctorTestHelper<Stencils::Moore<3, 1> >().testSplittedTraversal(3);
    }

    void testVonNeumann2D()
    {
        UpdateFunctorTestHelper<Stencils::VonNeumann<2, 1> >().testSimple(3);
        UpdateFunctorTestHelper<Stencils::VonNeumann<2, 1> >().testSplittedTraversal(3);
    }

    void testVonNeumann3D()
    {
        UpdateFunctorTestHelper<Stencils::VonNeumann<3, 1> >().testSimple(3);
        UpdateFunctorTestHelper<Stencils::VonNeumann<3, 1> >().testSplittedTraversal(3);
    }

    void testStructOfArrays()
    {
        CoordBox<3> box(Coord<3>(0, 0, 0), Coord<3>(30, 20, 10));
        SoAGrid<MySoATestCell, MySoATestCell::Topology> gridOld(box, MySoATestCell(47.0), MySoATestCell(1));
        SoAGrid<MySoATestCell, MySoATestCell::Topology> gridNew(box, MySoATestCell(11.0), MySoATestCell(0));
        Streak<3> streak(Coord<3>(5, 2, 1), 27);

        std::cout << "\n";
        std::cout << "---------------------------------------\n";
        UpdateFunctor<MySoATestCell>()(streak, streak.origin, gridOld, &gridNew, 0);
        std::cout << "---------------------------------------\n";

        // Coord<3> c(1, 2, 3);
        // TS_ASSERT_EQUALS(gridNew.get(c).temp, 11.0);
        // gridNew.set(c, gridOld.get(c));
        // gridNew.set(c, MySoATestCell(12.34, true));
        // TS_ASSERT_EQUALS(gridNew.get(c).temp, 12.34);
    }

private:
    template<typename CELL>
    void checkSelector(const std::string& line, int repeats)
    {
        Streak<2> streak(Coord<2>(2, 1), 10);
        Coord<2> dim(20, 10);
        int nanoStep = 0;

        Grid<CELL> gridOld(dim);
        Grid<CELL> gridNew(dim);

        UpdateFunctor<CELL>()(streak, streak.origin, gridOld, &gridNew, nanoStep);

        std::vector<char> message(1024 * 16, 0);
        myLog.read(&message[0], 1024 * 16);
        std::string expected = "";
        for (int i = 0; i < repeats; ++i) {
            expected += line;
        }

        TS_ASSERT_EQUALS(expected, std::string(&message[0]));
        myLog.clear();
    }
};

}
