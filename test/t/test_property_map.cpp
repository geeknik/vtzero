
#include <test.hpp>

#include <vtzero/builder.hpp>

#ifdef VTZERO_TEST_WITH_VARIANT
# include <boost/variant.hpp>
typedef boost::make_recursive_variant<
        std::string,
        float,
        double,
        int64_t,
        uint64_t,
        bool,
        std::vector<boost::recursive_variant_>,
        std::unordered_map<std::string, boost::recursive_variant_>
    >::type variant_type;
#endif

#include <map>
#include <string>
#include <unordered_map>

TEST_CASE("property map") {
    vtzero::tile_builder tile;
    vtzero::layer_builder layer_points{tile, "points"};
    {
        vtzero::point_feature_builder fbuilder{layer_points};
        fbuilder.set_id(1);
        fbuilder.add_points(1);
        fbuilder.set_point(10, 10);
        fbuilder.add_property("foo", "bar");
        fbuilder.add_property("x", "y");
        fbuilder.add_property("abc", "def");
        fbuilder.commit();
    }

    std::string data = tile.serialize();

    vtzero::vector_tile vt{data};
    REQUIRE(vt.count_layers() == 1);
    auto layer = vt.next_layer();
    REQUIRE(layer.valid());
    REQUIRE(layer.num_features() == 1);

    const auto feature = layer.next_feature();
    REQUIRE(feature.valid());
    REQUIRE(feature.num_properties() == 3);

#ifdef VTZERO_TEST_WITH_VARIANT
    SECTION("std::unordered_map") {
        auto map = vtzero::create_properties_map<variant_type>(feature);

        REQUIRE(map.size() == 3);
        boost::apply_visitor(prop_visitor{"bar"}, map["foo"]);
        boost::apply_visitor(prop_visitor{"y"}, map["x"]);
        boost::apply_visitor(prop_visitor{"def"}, map["abc"]);
    }
#endif
}

