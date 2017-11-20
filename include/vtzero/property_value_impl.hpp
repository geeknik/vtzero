#pragma once

#include "property_value.hpp"
#include "types.hpp"

namespace vtzero {
    
    template <typename TVariant, 
              typename TMapping,
              typename TMap>
    TMap create_properties_map(const vtzero::property_map & pm) {
        using TKey = typename TMap::key_type;
        TMap map;

        pm.for_each_property([&](property&& p) {
            map.emplace(TKey(p.key()), convert_property_value<TVariant, TMapping>(p.value()));
            return true;
        });

        return map;
    }
    
    template <typename TVariant,
              typename TMapping,
              typename TList>
    TList create_properties_list(const vtzero::property_list& pl) {
        TList list;

        pl.for_each_value([&](property_value && pv) {
            list.emplace_back(convert_property_value<TVariant, TMapping>(pv));
            return true;
        });

        return list;
    }

} // end ns vtzero
