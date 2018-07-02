#ifndef VTZERO_TYPES_HPP
#define VTZERO_TYPES_HPP

/*****************************************************************************

vtzero - Minimalistic vector tile decoder and encoder in C++.

This file is from https://github.com/mapbox/vtzero where you can find more
documentation.

*****************************************************************************/

#include "exception.hpp"

#include <protozero/pbf_reader.hpp>

#include <cassert>

// @cond internal
// Wrappers for assert() used for testing
#ifndef vtzero_assert
# define vtzero_assert(x) assert(x)
#endif
#ifndef vtzero_assert_in_noexcept_function
# define vtzero_assert_in_noexcept_function(x) assert(x)
#endif
// @endcond

/**
 * @file types.hpp
 *
 * @brief Contains the declaration of low-level types.
 */

/**
 * @brief All parts of the vtzero header-only library are in this namespace.
 */
namespace vtzero {

    class layer;
    class property;
    class property_value;

    /**
     * Using data_view class from protozero. See the protozero documentation
     * on how to change this to use a different implementation.
     * https://github.com/mapbox/protozero/blob/master/doc/advanced.md#protozero_use_view
     */
    using data_view = protozero::data_view;

    // based on https://github.com/mapbox/vector-tile-spec/blob/master/2.1/vector_tile.proto

    /// The geometry type as specified in the vector tile spec
    enum class GeomType {
        UNKNOWN    = 0,
        POINT      = 1,
        LINESTRING = 2,
        POLYGON    = 3,
        SPLINE     = 4
    };

    /**
     * Return the name of a GeomType (for debug output etc.)
     */
    inline const char* geom_type_name(GeomType type) noexcept {
        static const char* names[] = {
            "unknown", "point", "linestring", "polygon"
        };
        return names[static_cast<int>(type)]; // NOLINT clang-tidy: cppcoreguidelines-pro-bounds-constant-array-index
    }

    /// The property value type as specified in the vector tile spec
    enum class property_value_type : protozero::pbf_tag_type {
        string_value = 1,
        float_value  = 2,
        double_value = 3,
        int_value    = 4,
        uint_value   = 5,
        sint_value   = 6,
        bool_value   = 7,
        map_value    = 8,
        list_value   = 9
    };

    /**
     * Return the name of a property value type (for debug output etc.)
     */
    inline const char* property_value_type_name(property_value_type type) noexcept {
        static const char* names[] = {
            "", "string", "float", "double", "int", "uint", "sint", "bool"
        };
        return names[static_cast<int>(type)]; // NOLINT clang-tidy: cppcoreguidelines-pro-bounds-constant-array-index
    }

    namespace detail {

        enum class pbf_tile : protozero::pbf_tag_type {
            layers = 3
        };

        enum class pbf_layer : protozero::pbf_tag_type {
            name       =  1,
            features   =  2,
            keys       =  3,
            values     =  4,
            extent     =  5,
            dimensions =  6,
            version    = 15
        };

        enum class pbf_feature : protozero::pbf_tag_type {
            id       = 1,
            tags     = 2,
            type     = 3,
            geometry = 4,
            knots    = 5
        };

        using pbf_value = property_value_type;

    } // namespace detail

    /// property value type holding a reference to a string
    struct string_value_type {

        /// the underlying storage type
        using type = data_view;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::string_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::length_delimited;
        /// @endcond

        /// value
        data_view value{};

        /// Construct empty string_value_type
        constexpr string_value_type() noexcept = default;

        /// Construct string_value_type
        explicit constexpr string_value_type(data_view v) :
            value(v) {
        }

    }; // struct string_value_type

    /// property value type holding a float
    struct float_value_type {

        /// the underlying storage type
        using type = float;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::float_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::fixed32;
        /// @endcond

        /// value
        float value = 0.0f;

        /// Construct float_value_type with value 0.0
        constexpr float_value_type() noexcept = default;

        /// Construct float_value_type
        explicit constexpr float_value_type(float v) :
            value(v) {
        }

    }; // struct float_value_type

    /// property value type holding a double
    struct double_value_type {

        /// the underlying storage type
        using type = double;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::double_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::fixed64;
        /// @endcond

        /// value
        double value = 0.0;

        /// Construct double_value_type with value 0.0
        constexpr double_value_type() noexcept = default;

        /// Construct double_value_type
        explicit constexpr double_value_type(double v) :
            value(v) {
        }

    }; // struct double_value_type

    /// property value type holding an int
    struct int_value_type {

        /// the underlying storage type
        using type = int64_t;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::int_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::varint;
        /// @endcond

        /// value
        int64_t value = 0;

        /// Construct int_value_type with value 0
        constexpr int_value_type() noexcept = default;

        /// Construct int_value_type
        explicit constexpr int_value_type(int64_t v) :
            value(v) {
        }

    }; // struct int_value_type

    /// property value type holding a uint
    struct uint_value_type {

        /// the underlying storage type
        using type = uint64_t;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::uint_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::varint;
        /// @endcond

        /// value
        uint64_t value = 0;

        /// Construct uint_value_type with value 0
        constexpr uint_value_type() noexcept = default;

        /// Construct uint_value_type
        explicit constexpr uint_value_type(uint64_t v) :
            value(v) {
        }

    }; // struct uint_value_type

    /// property value type holding an sint
    struct sint_value_type {

        /// the underlying storage type
        using type = int64_t;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::sint_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::varint;
        /// @endcond

        /// value
        int64_t value = 0;

        /// Construct sint_value_type with value 0
        constexpr sint_value_type() noexcept = default;

        /// Construct sint_value_type
        explicit constexpr sint_value_type(int64_t v) :
            value(v) {
        }

    }; // struct sint_value_type

    /// property value type holding a bool
    struct bool_value_type {

        /// the underlying storage type
        using type = bool;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::bool_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::varint;
        /// @endcond

        /// value
        bool value = false;

        /// Construct bool_value_type with false value
        constexpr bool_value_type() noexcept = default;

        /// Construct bool_value_type
        explicit constexpr bool_value_type(bool v) :
            value(v) {
        }

    }; // struct bool_value_type

    /**
     * This class wraps the uint32_t used for looking up keys/values in the
     * key/values tables.
     */
    class index_value {

        static const uint32_t invalid_value = std::numeric_limits<uint32_t>::max();

        uint32_t m_value = invalid_value;

    public:

        /// Default construct to an invalid value.
        constexpr index_value() noexcept = default;

        /// Construct with the given value.
        constexpr index_value(uint32_t value) noexcept : // NOLINT (non-explicit on purpose)
            m_value(value) {
        }

        /**
         * Is this index value valid? Index values are valid if they have
         * been initialized with something other than the default constructor.
         */
        constexpr bool valid() const noexcept {
            return m_value != invalid_value;
        }

        /**
         * Get the value.
         *
         * @pre Must be valid.
         */
        uint32_t value() const noexcept {
            vtzero_assert_in_noexcept_function(valid());
            return m_value;
        }

    }; // class index_value

    /// Index values are equal if their values are.
    inline bool operator==(const index_value lhs, const index_value rhs) noexcept {
        return lhs.value() == rhs.value();
    }

    /// Index values are not equal if their values are not equal.
    inline bool operator!=(const index_value lhs, const index_value rhs) noexcept {
        return lhs.value() != rhs.value();
    }

    /**
     * This class holds two index_values, one for a key and one for a value.
     */
    class index_value_pair {

        index_value m_key{};
        index_value m_value{};

    public:

        /// Default construct to an invalid value.
        constexpr index_value_pair() noexcept = default;

        /// Construct with the given values.
        constexpr index_value_pair(index_value key, index_value value) noexcept :
            m_key(key),
            m_value(value) {
        }

        /**
         * Is this index value pair valid? Index values pairs are valid if
         * both the key and the value index value are valid.
         */
        constexpr bool valid() const noexcept {
            return m_key.valid() && m_value.valid();
        }

        /**
         * Is this index value pair valid? Index values pairs are valid if
         * both the key and the value index value are valid.
         */
        constexpr explicit operator bool() const noexcept {
            return valid();
        }

        /// Get the key index value.
        constexpr index_value key() const noexcept {
            return m_key;
        }

        /// Get the value index value.
        constexpr index_value value() const noexcept {
            return m_value;
        }

    }; // class index_value_pair

    /**
     * The geometry class holds a geometry type and a reference to the data
     * defining this geometry. To actually decode these geometries, use the
     * decode_point_geometry(), decode_linestring_geometry(), and
     * decode_polygon_geometry() classes.
     */
    class geometry {

        data_view m_data{};
        data_view m_knots{};
        GeomType m_type = GeomType::UNKNOWN;

    public:

        /**
         * A forward iterator yielding 32bit unsigned integers with the
         * geometry encoded according to spec 4.3.
         */
        using const_iterator = protozero::pbf_reader::const_uint32_iterator;
        using knots_const_iterator = protozero::const_fixed_iterator<double>;

        /// Default construct to an invalid value.
        constexpr geometry() noexcept = default;

        /// Construct with the given values.
        constexpr geometry(data_view data, data_view knots, GeomType type) noexcept :
            m_data(data),
            m_knots(knots),
            m_type(type) {
        }

        /// The data of this geometry
        constexpr data_view data() const noexcept {
            return m_data;
        }

        /// The data of this geometry
        constexpr data_view knots() const noexcept {
            return m_knots;
        }

        /// The type of this geometry
        constexpr GeomType type() const noexcept {
            return m_type;
        }

        /// Return iterator to the beginning of the data.
        const_iterator begin() const noexcept {
            return {m_data.data(), m_data.data() + m_data.size()};
        }

        /// Return iterator to one past the end of the data.
        const_iterator end() const noexcept {
            return {m_data.data() + m_data.size(), m_data.data() + m_data.size()};
        }

        knots_const_iterator knots_begin() const noexcept {
            return knots_const_iterator{m_knots.data()};
        }

        knots_const_iterator knots_end() const noexcept {
            return knots_const_iterator{m_knots.data() + m_knots.size()};
        }

    }; // class geometry

    class property_map {

        using uint32_iterator = protozero::pbf_reader::const_uint32_iterator;
        using uint32_iterator_range = protozero::iterator_range<uint32_iterator>;

        uint32_iterator_range m_properties{};
        uint32_iterator m_property_iterator{};
        const layer* m_layer = nullptr;
        std::size_t m_num_properties = 0;

    public:

        property_map() = default;

        property_map(const layer* layer) noexcept :
            m_layer(layer) {
        }

        property_map(const layer* layer, uint32_iterator_range properties) :
            m_properties(properties),
            m_property_iterator(m_properties.begin()),
            m_layer(layer),
            m_num_properties(0) {
            set_size();
        }

        void set_size() {
            const auto size = m_properties.size();
            if (size % 2 != 0) {
                throw format_exception{"unpaired property key/value indexes (spec 4.4)"};
            }
            m_num_properties = size / 2;
        }

        void initialize(uint32_iterator_range properties) {
            if (m_properties.begin() != uint32_iterator{}) {
                throw format_exception{"Feature has more than one tags field"};
            }
            m_properties = properties;
            m_property_iterator = m_properties.begin();

            set_size();
        }

        bool empty() const noexcept {
            return m_num_properties == 0;
        }

        std::size_t size() const noexcept {
            return m_num_properties;
        }

        property next();

        index_value_pair next_indexes() {
            if (m_property_iterator == m_properties.end()) {
                return {};
            }
            const auto ki = *m_property_iterator++;
            const auto vi = *m_property_iterator++;
            return {ki, vi};
        }

        void reset() noexcept {
            m_property_iterator = m_properties.begin();
        }

        template <typename TFunc>
        bool for_each_property(TFunc&& func) const;

    }; // class property_map

    class property_list {

        using uint32_iterator = protozero::pbf_reader::const_uint32_iterator;
        using uint32_iterator_range = protozero::iterator_range<uint32_iterator>;

        uint32_iterator_range m_properties{};
        uint32_iterator m_property_iterator{};
        const layer* m_layer = nullptr;
        std::size_t m_num_properties = 0;

    public:

        property_list() = default;

        property_list(const layer* layer, uint32_iterator_range properties) :
            m_properties(properties),
            m_property_iterator(m_properties.begin()),
            m_layer(layer),
            m_num_properties(m_properties.size()) {
        }

        bool empty() const noexcept {
            return m_num_properties == 0;
        }

        std::size_t size() const noexcept {
            return m_num_properties;
        }

        property_value next();

        index_value next_index() {
            if (m_property_iterator == m_properties.end()) {
                return {};
            }
            const auto vi = *m_property_iterator++;
            return {vi};
        }

        void reset() noexcept {
            m_property_iterator = m_properties.begin();
        }

        template <typename TFunc>
        bool for_each_value(TFunc&& func) const;

    }; // class property_list

    /// property value type holding a reference to a map
    struct map_value_type {

        /// the underlying storage type
        using type = property_map;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::map_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::length_delimited;
        /// @endcond

        /// value
        property_map value{};

        /// Construct empty string_value_type
        map_value_type() = default;

        /// Construct string_value_type
        explicit constexpr map_value_type(property_map v) :
            value(v) {
        }

    }; // struct map_value_type

    /// property value type holding a reference to a list
    struct list_value_type {

        /// the underlying storage type
        using type = property_list;

        /// @cond internal
        constexpr static const property_value_type pvtype = property_value_type::list_value;
        constexpr static const protozero::pbf_wire_type wire_type = protozero::pbf_wire_type::length_delimited;
        /// @endcond

        /// value
        property_list value{};

        /// Construct empty string_value_type
        list_value_type() = default;

        /// Construct string_value_type
        explicit constexpr list_value_type(property_list v) :
            value(v) {
        }

    }; // struct list_value_type

} // namespace vtzero

#endif // VTZERO_TYPES_HPP
