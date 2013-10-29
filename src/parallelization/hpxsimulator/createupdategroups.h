#ifndef LIBGEODECOMP_PARALLELIZATION_HPXSIMULATOR_CREATEUPDATEGROUPS_H
#define LIBGEODECOMP_PARALLELIZATION_HPXSIMULATOR_CREATEUPDATEGROUPS_H

#include <libgeodecomp/config.h>
#ifdef LIBGEODECOMP_FEATURE_HPX

#include <hpx/config.hpp>
#include <hpx/hpx_fwd.hpp>
#include <hpx/util/locality_result.hpp>
#include <hpx/runtime/actions/plain_action.hpp>

#include <libgeodecomp/io/initializer.h>

#include <boost/range/algorithm/copy.hpp>

#include <utility>
#include <vector>

namespace LibGeoDecomp {
namespace HpxSimulator {
namespace Implementation {

    struct OvercommitFunctor
    {
        double overcommitFactor;

        std::size_t operator()() const
        {
            return std::ceil(hpx::get_os_thread_count() * overcommitFactor);
        }

        template <typename ARCHIVE>
        void serialize(ARCHIVE& ar, unsigned)
        {
            ar & overcommitFactor;
        }
    };

typedef
    std::pair<std::size_t, std::vector<hpx::util::remote_locality_result> >
    CreateUpdateGroupsReturnType;

std::pair<std::size_t, std::vector<hpx::util::remote_locality_result> >
createUpdateGroups(
    std::vector<hpx::id_type> localities,
    hpx::components::component_type type,
    const hpx::util::function<std::size_t()>& numUpdateGroups);

HPX_DEFINE_PLAIN_ACTION(createUpdateGroups, CreateUpdateGroupsAction);

} // namespace Implementation

template <class UPDATEGROUP>
inline std::vector<UPDATEGROUP> createUpdateGroups(
    const hpx::util::function<std::size_t()>& numUpdateGroups
)
{
    hpx::components::component_type type =
        hpx::components::get_component_type<typename UPDATEGROUP::ComponentType>();

    std::vector<hpx::id_type> localities = hpx::find_all_localities(type);

    hpx::id_type id = localities[0];
    hpx::future<std::pair<std::size_t, std::vector<hpx::util::remote_locality_result> > >
        asyncResult = hpx::async<Implementation::CreateUpdateGroupsAction>(
            id, boost::move(localities), type, numUpdateGroups);

    std::vector<UPDATEGROUP> components;

    std::pair<std::size_t, std::vector<hpx::util::remote_locality_result> >
        result(boost::move(asyncResult.move()));

    std::size_t numComponents = result.first;
    components.reserve(numComponents);

    std::vector<hpx::util::locality_result> res;
    res.reserve(result.second.size());
    boost::copy(result.second, std::back_inserter(res));
    boost::copy(hpx::util::locality_results(res), std::back_inserter(components));

    return components;
}

template <class UPDATEGROUP>
inline std::vector<UPDATEGROUP> createUpdateGroups(float overcommitFactor)
{
    Implementation::OvercommitFunctor f = {overcommitFactor};
    return createUpdateGroups<UPDATEGROUP>(f);
}

}
}

HPX_REGISTER_PLAIN_ACTION_DECLARATION(
    LibGeoDecomp::HpxSimulator::Implementation::CreateUpdateGroupsAction
)

HPX_REGISTER_BASE_LCO_WITH_VALUE_DECLARATION(
    LibGeoDecomp::HpxSimulator::Implementation::CreateUpdateGroupsReturnType,
    hpx_base_lco_std_pair_std_size_t_std_vector_hpx_util_remote_locality_result
)

HPX_UTIL_REGISTER_FUNCTION_DECLARATION(
    std::size_t(),
    LibGeoDecomp::HpxSimulator::Implementation::OvercommitFunctor,
    LibGeoDecompHpxSimulatorImplementationOvercommitFunctor)

HPX_UTIL_REGISTER_FUNCTION_DECLARATION(
    std::size_t(),
    hpx::util::function<std::size_t()>,
    LibGeoDecompHpxSimulatorImplementationFunction)

#endif
#endif
