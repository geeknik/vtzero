#ifndef VTZERO_PROPERTY_VALUE_IMPL_HPP
#define VTZERO_PROPERTY_VALUE_IMPL_HPP

/*****************************************************************************

vtzero - Minimalistic vector tile decoder and encoder in C++.

This file is from https://github.com/mapbox/vtzero where you can find more
documentation.

*****************************************************************************/

#include "property_value.hpp"
#include "types.hpp"

namespace vtzero {

    template <typename TVariant, typename TMapping, typename TMap>
    TMap create_properties_map(const vtzero::property_map& pm) {
        using key_type = typename TMap::key_type;
        TMap map;

        pm.for_each_property([&](property&& p) {
            map.emplace(key_type{p.key()}, convert_property_value<TVariant, TMapping>(p.value()));
            return true;
        });

        return map;
    }

    template <typename TVariant, typename TMapping, typename TList>
    TList create_properties_list(const vtzero::property_list& pl) {
        TList list;

        pl.for_each_value([&](property_value&& pv) {
            list.emplace_back(convert_property_value<TVariant, TMapping>(pv));
            return true;
        });

        return list;
    }

} // namespace vtzero

#endif // VTZERO_PROPERTY_VALUE_IMPL_HPP
