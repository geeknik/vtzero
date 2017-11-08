
#include <test.hpp>

#include <vtzero/builder.hpp>
#include <vtzero/index.hpp>

#include <cstdint>
#include <string>
#include <type_traits>

template <typename T>
struct movable_not_copyable {
    constexpr static bool value = !std::is_copy_constructible<T>::value &&
                                  !std::is_copy_assignable<T>::value    &&
                                   std::is_nothrow_move_constructible<T>::value &&
                                   std::is_nothrow_move_assignable<T>::value;
};

static_assert(movable_not_copyable<vtzero::tile_builder>::value, "tile_builder should be nothrow movable, but not copyable");
static_assert(movable_not_copyable<vtzero::point_feature_builder>::value, "point_feature_builder should be nothrow movable, but not copyable");
static_assert(movable_not_copyable<vtzero::linestring_feature_builder>::value, "linestring_feature_builder should be nothrow movable, but not copyable");
static_assert(movable_not_copyable<vtzero::polygon_feature_builder>::value, "polygon_feature_builder should be nothrow movable, but not copyable");
static_assert(movable_not_copyable<vtzero::geometry_feature_builder>::value, "geometry_feature_builder should be nothrow movable, but not copyable");

TEST_CASE("Create tile from existing layers") {
    const auto buffer = load_test_tile();
    vtzero::vector_tile tile{buffer};

    vtzero::tile_builder tbuilder;

    SECTION("add_existing_layer(layer)") {
        while (auto layer = tile.next_layer()) {
            tbuilder.add_existing_layer(layer);
        }
    }

    SECTION("add_existing_layer(data_view)") {
        while (auto layer = tile.next_layer()) {
            tbuilder.add_existing_layer(layer.data());
        }
    }

    const std::string data = tbuilder.serialize();

    REQUIRE(data == buffer);
}

TEST_CASE("Create layer based on existing layer") {
    const auto buffer = load_test_tile();
    vtzero::vector_tile tile{buffer};
    const auto layer = tile.get_layer_by_name("place_label");

    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, layer};
    vtzero::point_feature_builder fbuilder{lbuilder};
    fbuilder.set_id(42);
    fbuilder.add_point(10, 20);
    fbuilder.commit();

    const std::string data = tbuilder.serialize();
    vtzero::vector_tile new_tile{data};
    const auto new_layer = new_tile.next_layer();
    REQUIRE(std::string(new_layer.name()) == "place_label");
    REQUIRE(new_layer.version() == 1);
    REQUIRE(new_layer.extent() == 4096);
}

TEST_CASE("Create layer and add keys/values") {
    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, "name"};

    const auto ki1 = lbuilder.add_key_without_dup_check("key1");
    const auto ki2 = lbuilder.add_key("key2");
    const auto ki3 = lbuilder.add_key("key1");

    REQUIRE(ki1 != ki2);
    REQUIRE(ki1 == ki3);

    const auto vi1 = lbuilder.add_value_without_dup_check(vtzero::encoded_property_value{"value1"});
    vtzero::encoded_property_value value2{"value2"};
    const auto vi2 = lbuilder.add_value_without_dup_check(vtzero::property_value{value2.data()});

    const auto vi3 = lbuilder.add_value(vtzero::encoded_property_value{"value1"});
    const auto vi4 = lbuilder.add_value(vtzero::encoded_property_value{19});
    const auto vi5 = lbuilder.add_value(vtzero::encoded_property_value{19.0});
    const auto vi6 = lbuilder.add_value(vtzero::encoded_property_value{22});
    vtzero::encoded_property_value nineteen{19};
    const auto vi7 = lbuilder.add_value(vtzero::property_value{nineteen.data()});

    REQUIRE(vi1 != vi2);
    REQUIRE(vi1 == vi3);
    REQUIRE(vi1 != vi4);
    REQUIRE(vi1 != vi5);
    REQUIRE(vi1 != vi6);
    REQUIRE(vi4 != vi5);
    REQUIRE(vi4 != vi6);
    REQUIRE(vi4 == vi7);
}

TEST_CASE("Committing a feature succeeds after a geometry was added") {
    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, "test"};

    { // explicit commit after geometry
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(1);
        fbuilder.add_point(10, 10);
        fbuilder.commit();
    }

    { // explicit commit after properties
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        fbuilder.add_point(10, 10);
        fbuilder.add_property("foo", vtzero::encoded_property_value{"bar"});
        fbuilder.commit();
    }

    { // implicit commit after geometry
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(3);
        fbuilder.add_point(10, 10);
    }

    { // implicit commit after properties
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(4);
        fbuilder.add_point(10, 10);
        fbuilder.add_property("foo", vtzero::encoded_property_value{"bar"});
    }

    { // multiple commits is okay
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(5);
        fbuilder.add_point(10, 10);
        fbuilder.add_property("foo", vtzero::encoded_property_value{"bar"});
        fbuilder.commit();
        fbuilder.commit();
    }

    const std::string data = tbuilder.serialize();

    vtzero::vector_tile tile{data};
    auto layer = tile.next_layer();

    uint64_t n = 1;
    while (auto feature = layer.next_feature()) {
        REQUIRE(feature.id() == n++);
    }
}

TEST_CASE("Committing a feature fails with assert if no geometry was added") {
    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, "test"};

    SECTION("explicit immediate commit") {
        vtzero::point_feature_builder fbuilder{lbuilder};
        REQUIRE_ASSERT(fbuilder.commit());
    }

    SECTION("explicit commit after setting id") {
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        REQUIRE_ASSERT(fbuilder.commit());
    }

    SECTION("implicit immediate commit") {
        const auto lambda = [&](){
            vtzero::point_feature_builder fbuilder{lbuilder};
        };
        REQUIRE_ASSERT(lambda());
    }
    SECTION("implicit commit after setting id") {
        const auto lambda = [&](){
            vtzero::point_feature_builder fbuilder{lbuilder};
            fbuilder.set_id(2);
        };
        REQUIRE_ASSERT(lambda());
    }
}

TEST_CASE("Rollback feature") {
    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, "test"};

    {
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(1);
        fbuilder.add_point(10, 10);
        fbuilder.commit();
    }

    { // immediate rollback
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        fbuilder.rollback();
    }

    { // rollback after setting id
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        fbuilder.rollback();
    }

    { // rollback after geometry
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        fbuilder.add_point(20, 20);
        fbuilder.rollback();
    }

    { // rollback after properties
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(2);
        fbuilder.add_point(20, 20);
        fbuilder.add_property("foo", vtzero::encoded_property_value{"bar"});
        fbuilder.rollback();
    }

    {
        vtzero::point_feature_builder fbuilder{lbuilder};
        fbuilder.set_id(3);
        fbuilder.add_point(30, 30);
    }

    const std::string data = tbuilder.serialize();

    vtzero::vector_tile tile{data};
    auto layer = tile.next_layer();

    auto feature = layer.next_feature();
    REQUIRE(feature.id() == 1);

    feature = layer.next_feature();
    REQUIRE(feature.id() == 3);

    feature = layer.next_feature();
    REQUIRE_FALSE(feature);
}

TEST_CASE("Rolling back an already committed feature fails with asserts") {
    vtzero::tile_builder tbuilder;
    vtzero::layer_builder lbuilder{tbuilder, "test"};

    vtzero::point_feature_builder fbuilder{lbuilder};
    fbuilder.set_id(1);
    fbuilder.add_point(10, 10);
    fbuilder.commit();
    REQUIRE_THROWS_AS(fbuilder.rollback(), const assert_error&);
}

