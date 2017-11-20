#ifndef VTZERO_PROPERTY_VALUE_HPP
#define VTZERO_PROPERTY_VALUE_HPP

/*****************************************************************************

vtzero - Tiny and fast vector tile decoder and encoder in C++.

This file is from https://github.com/mapbox/vtzero where you can find more
documentation.

*****************************************************************************/

/**
 * @file property_value.hpp
 *
 * @brief Contains the property_value class.
 */

#include "exception.hpp"
#include "types.hpp"

#include <protozero/pbf_message.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>
#include <unordered_map>

namespace vtzero {

    /**
     * A view of a vector tile property value.
     *
     * Doesn't hold any data itself.
     */
    class property_value {

        data_view m_value{};
        const layer* m_layer = nullptr;

        static bool check_tag_and_type(protozero::pbf_tag_type tag, protozero::pbf_wire_type type) noexcept {
            static constexpr const std::array<protozero::pbf_wire_type, 9> types{{
                string_value_type::wire_type,
                float_value_type::wire_type,
                double_value_type::wire_type,
                int_value_type::wire_type,
                uint_value_type::wire_type,
                sint_value_type::wire_type,
                bool_value_type::wire_type,
                map_value_type::wire_type,
                list_value_type::wire_type,
            }};

            if (tag < 1 || tag > types.size()) {
                return false;
            }

            return types[tag - 1] == type; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }

        data_view get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, string_value_type /* dummy */) const {
            return value_message.get_view();
        }

        float get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, float_value_type /* dummy */) const {
            return value_message.get_float();
        }

        double get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, double_value_type /* dummy */) const {
            return value_message.get_double();
        }

        int64_t get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, int_value_type /* dummy */) const {
            return value_message.get_int64();
        }

        uint64_t get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, uint_value_type /* dummy */) const {
            return value_message.get_uint64();
        }

        int64_t get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, sint_value_type /* dummy */) const {
            return value_message.get_sint64();
        }

        bool get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, bool_value_type /* dummy */) const {
            return value_message.get_bool();
        }

        property_map get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, map_value_type /* dummy */) const {
            return property_map{ m_layer, value_message.get_packed_uint32() };
        }
        
        property_list get_value_impl(protozero::pbf_message<detail::pbf_value>& value_message, list_value_type /* dummy */) const {
            return property_list{ m_layer, value_message.get_packed_uint32() };
        }

        template <typename T>
        typename T::type get_value() const {
            vtzero_assert(valid());
            protozero::pbf_message<detail::pbf_value> value_message{m_value};

            typename T::type result{};
            bool has_result = false;
            while (value_message.next(T::pvtype, T::wire_type)) {
                result = get_value_impl(value_message, T());
                has_result = true;
            }

            if (has_result) {
                return result;
            }

            throw type_exception{};
        }

    public:

        /**
         * The default constructor creates an invalid (empty) property_value.
         */
        constexpr property_value() noexcept = default;

        /**
         * Create a (valid) property_value from a data_view and layer
         */
        explicit constexpr property_value(const data_view value) noexcept :
            m_value(value) {
        }

        /**
         * Create a (valid) property_value from a data_view and layer
         */
        explicit constexpr property_value(const data_view value, const layer* layer) noexcept :
            m_value(value),
            m_layer(layer) {
        }

        /**
         * Is this a valid property value? Property values are valid if they
         * were constructed using the non-default constructor.
         */
        constexpr bool valid() const noexcept {
            return m_value.data() != nullptr;
        }

        /**
         * Is this a valid property_value? Properties are valid if they were
         * constructed using the non-default constructor.
         */
        explicit constexpr operator bool() const noexcept {
            return valid();
        }

        /**
         * Get the type of this property.
         *
         * @pre @code valid() @endcode
         * @throws format_exception if the encoding is invalid
         */
        property_value_type type() const {
            vtzero_assert(valid());
            protozero::pbf_message<detail::pbf_value> value_message{m_value};
            if (value_message.next()) {
                const auto tag_val = static_cast<protozero::pbf_tag_type>(value_message.tag());
                if (!check_tag_and_type(tag_val, value_message.wire_type())) {
                    throw format_exception{"illegal property value type"};
                }
                return value_message.tag();
            }
            throw format_exception{"missing tag value"};
        }

        /**
         * Get the internal data_view this object was constructed with.
         */
        constexpr data_view data() const noexcept {
            return m_value;
        }

        /**
         * Get string value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than string.
         */
        data_view string_value() const {
            return get_value<string_value_type>();
        }

        /**
         * Get float value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than float.
         */
        float float_value() const {
            return get_value<float_value_type>();
        }

        /**
         * Get double value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than double.
         */
        double double_value() const {
            return get_value<double_value_type>();
        }

        /**
         * Get int value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than int.
         */
        std::int64_t int_value() const {
            return get_value<int_value_type>();
        }

        /**
         * Get uint value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than uint.
         */
        std::uint64_t uint_value() const {
            return get_value<uint_value_type>();
        }

        /**
         * Get sint value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than sint.
         */
        std::int64_t sint_value() const {
            return get_value<sint_value_type>();
        }

        /**
         * Get bool value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than bool.
         */
        bool bool_value() const {
            return get_value<bool_value_type>();
        }
        
        /**
         * Get map value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than map.
         */
        property_map map_value() const {
            return get_value<map_value_type>();
        }
        
        /**
         * Get list value of this object.
         *
         * @pre @code valid() @endcode
         * @throws type_exception if the type of this property value is
         *                        something other than list.
         */
        property_list list_value() const {
            return get_value<list_value_type>();
        }

    }; // class property_value

    /// property_values are equal if they contain the same data.
    inline constexpr bool operator==(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() == rhs.data();
    }

    /// property_values are unequal if they do not contain the same data.
    inline constexpr bool operator!=(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() != rhs.data();
    }

    /// property_values are ordered in the same way as the underlying data
    inline bool operator<(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() < rhs.data();
    }

    /// property_values are ordered in the same way as the underlying data
    inline bool operator<=(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() <= rhs.data();
    }

    /// property_values are ordered in the same way as the underlying data
    inline bool operator>(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() > rhs.data();
    }

    /// property_values are ordered in the same way as the underlying data
    inline bool operator>=(const property_value lhs, const property_value rhs) noexcept {
        return lhs.data() >= rhs.data();
    }

    /**
     * Apply the value to a visitor.
     *
     * The visitor must have an overloaded call operator taking a single
     * argument of each of the types data_view, float, double, int64_t,
     * uint64_t, and bool. All call operators must return the same type which
     * will be the return type of this function.
     */
    template <typename V>
    decltype(std::declval<V>()(data_view{})) apply_visitor(V&& visitor, const property_value value) {
        switch (value.type()) {
            case property_value_type::string_value:
                return std::forward<V>(visitor)(value.string_value());
            case property_value_type::float_value:
                return std::forward<V>(visitor)(value.float_value());
            case property_value_type::double_value:
                return std::forward<V>(visitor)(value.double_value());
            case property_value_type::int_value:
                return std::forward<V>(visitor)(value.int_value());
            case property_value_type::uint_value:
                return std::forward<V>(visitor)(value.uint_value());
            case property_value_type::sint_value:
                return std::forward<V>(visitor)(value.sint_value());
            case property_value_type::map_value:
                return std::forward<V>(visitor)(value.map_value());
            case property_value_type::list_value:
                return std::forward<V>(visitor)(value.map_value());
            default: // case property_value_type::bool_value:
                return std::forward<V>(visitor)(value.bool_value());
        }
    }
    
    /**
     * Default mapping between the different types of a property_value to
     * the types needed for a variant. Derive from this class, overwrite
     * the types you want and use that class as second template parameter
     * in the convert_property_value class.
     */
    struct property_value_mapping {

        /// mapping for string type
        using string_type = std::string;

        /// mapping for float type
        using float_type = float;

        /// mapping for double type
        using double_type = double;

        /// mapping for int type
        using int_type = int64_t;

        /// mapping for uint type
        using uint_type = uint64_t;

        /// mapping for bool type
        using bool_type = bool;

        /// mapping for map
        template <typename Key, typename Value>
        using map_type = std::unordered_map<Key, Value>;

        /// mapping for list
        template <typename Value>
        using list_type = std::vector<Value>;

    }; // struct property_value_mapping
    
    template <typename TVariant, 
              typename TMapping = property_value_mapping,
              typename TMap = typename TMapping::template map_type<typename TMapping::string_type, TVariant>>
    TMap create_properties_map(const vtzero::property_map & pm);
    
    template <typename TVariant,
              typename TMapping = property_value_mapping,
              typename TList = typename TMapping::template list_type<TVariant>>
    TList create_properties_list(const vtzero::property_list& pl);


    namespace detail {

        template <typename TVariant, typename TMapping>
        struct convert_visitor {

            TVariant operator()(data_view value) const {
                return TVariant(typename TMapping::string_type(value));
            }

            TVariant operator()(float value) const {
                return TVariant(typename TMapping::float_type(value));
            }

            TVariant operator()(double value) const {
                return TVariant(typename TMapping::double_type(value));
            }

            TVariant operator()(int64_t value) const {
                return TVariant(typename TMapping::int_type(value));
            }

            TVariant operator()(uint64_t value) const {
                return TVariant(typename TMapping::uint_type(value));
            }

            TVariant operator()(bool value) const {
                return TVariant(typename TMapping::bool_type(value));
            }
            
            TVariant operator()(property_map const& value) const {
                return TVariant(create_properties_map<TVariant, TMapping>(value));
            }
            
            TVariant operator()(property_list const& value) const {
                return TVariant(create_properties_list<TVariant, TMapping>(value));
            }

        }; // struct convert_visitor

    } // namespace detail


    /**
     * Convert a property_value to a different (usually variant-based)
     * class.
     *
     * Usage: If you have a variant type like
     *
     * @code
     *   using variant_type = boost::variant<std::string, float, double, int64_t, uint64_t, bool>;
     * @endcode
     *
     * you can use
     * @code
     *   property_value x = ...;
     *   auto v = convert_property_value<variant_type>(x);
     * @endcode
     *
     * to convert the data.
     *
     * Usually your variant type has to support all of the following types:
     * std::string, float, double, int64_t, uint64_t, and bool. If your type
     * doesn't, you can add a second template parameter with a struct
     * containing the mapping between the vtzero types and your types:
     *
     * @code
     *   struct mapping : vtzero::property_value_mapping {
     *     using float_type = double; // convert all floats to doubles
     *     using bool_type = int; // convert all bools to ints
     *     // use default types for the rest
     *     // see the class vtzero::property_value_mapping for the defaults
     *   };
     *   property_value x = ...;
     *   auto v = convert_property_value<variant_type, mapping>(x);
     * @endcode
     *
     * @tparam TVariant The variant type to convert to.
     * @tparam TMapping A struct derived from property_value_mapping with the
     *         mapping for the types.
     * @param value The property value to convert.
     *
     */
    template <typename TVariant, typename TMapping = property_value_mapping>
    TVariant convert_property_value(const property_value value) {
        return apply_visitor(detail::convert_visitor<TVariant, TMapping>{}, value);
    }
    
} // namespace vtzero

#endif // VTZERO_PROPERTY_VALUE_HPP
