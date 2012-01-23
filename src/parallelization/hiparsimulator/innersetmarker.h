#include <libgeodecomp/config.h>
#ifdef LIBGEODECOMP_FEATURE_MPI
#ifndef _libgeodecomp_parallelization_hiparsimulator_innersetmarker_h_
#define _libgeodecomp_parallelization_hiparsimulator_innersetmarker_h_

#include <libgeodecomp/misc/region.h>
#include <libgeodecomp/misc/typetraits.h>
#include <libgeodecomp/parallelization/hiparsimulator/partitionmanager.h>

namespace LibGeoDecomp {
namespace HiParSimulator {

// fixme: remove
template<int DIM>
class InnerSetMarker
{
public:
    inline InnerSetMarker(const PartitionManager<DIM>& regionManager) :
        // fixme: this is dangerous as the pointer might become invalid
        regions(&regionManager.ownInnerSets)
    {}

    inline typename Region<DIM>::Iterator begin(const unsigned& i) const 
    {
        return (*regions)[i].begin();
    }
        
    inline typename Region<DIM>::Iterator end(const unsigned& i) const
    {
        return (*regions)[i].end();
    }

    inline StreakIterator<DIM> beginStreak(const unsigned& i) const 
    {
        return (*regions)[i].beginStreak();
    }
        
    inline StreakIterator<DIM> endStreak(const unsigned& i) const
    {
        return (*regions)[i].endStreak();
    }

    inline const Region<DIM>& region(const unsigned& i) const
    {
        return (*regions)[i];
    }

private:
    const SuperVector<Region<DIM> > *regions;
};

}

template<>
template<int DIM>
class ProvidesStreakIterator<HiParSimulator::InnerSetMarker<DIM> > : public boost::true_type {};

}

#endif

#endif
