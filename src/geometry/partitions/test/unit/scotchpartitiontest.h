#include <cxxtest/TestSuite.h>

#include <libgeodecomp/geometry/coordbox.h>
#include <libgeodecomp/geometry/partitions/scotchpartition.h>

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;
using namespace LibGeoDecomp;

namespace LibGeoDecomp {

class ScotchPartitionTest : public CxxTest::TestSuite
{
public:
    void testComplete2D()
    {
#ifdef LIBGEODECOMP_WITH_SCOTCH
        Coord<2> origin(0, 0);
        Coord<2> dimensions(512, 234);
        std::vector<std::size_t> weights;
        weights << 100 << 100 << 100 << 100;
        ScotchPartition<2> p(origin, dimensions, 0, weights);

        Region<2> expected0;

        expected0 << CoordBox<2>(Coord<2>(0,0), Coord<2>(512,234));

        Region<2> complete = p.getRegion(0) + p.getRegion(1) + p.getRegion(2) + p.getRegion(3);

        TS_ASSERT_EQUALS(expected0, complete);
#endif
     }

     void testOverlapse2D()
     {
#ifdef LIBGEODECOMP_WITH_SCOTCH
         Coord<2> origin(0, 0);
         Coord<2> dimensions(128, 231);
         std::vector<std::size_t> weights;
         weights << 100 << 100 << 100 << 100;
         ScotchPartition<2> p(origin, dimensions, 0, weights);

         Region<2> expected0;

         expected0 << CoordBox<2>(Coord<2>(0,0), Coord<2>(0,0));

         Region<2> cut = p.getRegion(0) & p.getRegion(1) & p.getRegion(2) & p.getRegion(3);

         TS_ASSERT_EQUALS(expected0, cut);
#endif
     }

    void testEqual3D()
    {
#ifdef LIBGEODECOMP_WITH_SCOTCH
        Coord<3> origin(0,0,0);
        Coord<3> dimensions(4,4,4);
        std::vector<std::size_t> weights;
        weights << 100 << 100 << 100 << 100;
        ScotchPartition<3> p(origin, dimensions, 0, weights);
        std::size_t sizeRegion0 = p.getRegion(0).size();
        std::size_t compSize;

        for(unsigned int i = 1 ; i < weights.size() ; ++i){
            compSize = p.getRegion(i).size();
            TS_ASSERT(sizeRegion0 == compSize ||
                      sizeRegion0 == compSize - 1 ||
                      sizeRegion0 == compSize + 1);
        }
#endif
    }


    void testComplete3D()
    {
#ifdef LIBGEODECOMP_WITH_SCOTCH
        Coord<3> origin(10, 10, 10);
        Coord<3> dimensions(123,26,27);
        std::vector<std::size_t> weights;
        weights << 100 << 100 << 100 << 100;
        ScotchPartition<3> p(origin, dimensions, 0, weights);
        Region<3> expected0;

        expected0 << CoordBox<3>(Coord<3>(10,10,10), Coord<3>(123,26,27));

        Region<3> complete = p.getRegion(0)
            + p.getRegion(1)
            + p.getRegion(2)
            + p.getRegion(3);

        TS_ASSERT_EQUALS(expected0, complete);
#endif
     }

     void testOverlapse3D()
     {
#ifdef LIBGEODECOMP_WITH_SCOTCH
        Coord<3> origin(0, 0);
        Coord<3> dimensions(28,231,52);
        std::vector<std::size_t> weights;
        weights << 100 << 100 << 100 << 100;
        ScotchPartition<3> p(origin, dimensions, 0, weights);

        Region<3> expected0;

        expected0 << CoordBox<3>(Coord<3>(0,0), Coord<3>(0,0));

        Region<3> cut = p.getRegion(0)
            & p.getRegion(1)
            & p.getRegion(2)
            & p.getRegion(3);

        TS_ASSERT_EQUALS(expected0, cut);
#endif
     }

};

}
