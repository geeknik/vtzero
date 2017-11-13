
#include <test.hpp>

#include <vtzero/geometry.hpp>

#include <cstdint>
#include <vector>

using container = std::vector<uint32_t>;
using knots_container = std::vector<double>;
using iterator = container::const_iterator;
using knots_iterator = knots_container::const_iterator;

class dummy_geom_handler {

    int value = 0;

public:

    void controlpoints_begin(const uint32_t /*count*/) noexcept {
        ++value;
    }

    void controlpoints_point(const vtzero::point<2> /*point*/) noexcept {
        value += 100;
    }

    void controlpoints_end() noexcept {
        value += 10000;
    }
    
    void knots_begin(const uint32_t /*count*/) noexcept {
        ++value;
    }
    
    void knots_value(double /*val*/) noexcept {
        value += 2;
    }

    void knots_end() noexcept {
        value += 200;
    }

    int result() const noexcept {
        return value;
    }
}; // class dummy_geom_handler

TEST_CASE("Calling decode_spline_geometry() with empty input") {
    const container g;
    const knots_container k;
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    dummy_geom_handler handler;
    decoder.decode_spline(dummy_geom_handler{});
    REQUIRE(handler.result() == 0);
}

TEST_CASE("Calling decode_spline_geometry() with a valid spline") {
    const container g = {9, 4, 4, 18, 0, 16, 16, 0};
    const knots_container k = {0.0, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    REQUIRE(decoder.decode_spline(dummy_geom_handler{}) == 10522);
}


TEST_CASE("Calling decode_spline_geometry() with a point geometry fails") {
    const container g = {9, 50, 34}; // this is a point geometry
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "expected LineTo command (spec 4.3.4.3)");
    }
}

TEST_CASE("Calling decode_spline_geometry() with a polygon geometry fails") {
    const container g = {9, 6, 12, 18, 10, 12, 24, 44, 15}; // this is a polygon geometry
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "additional data after end of geometry (spec 4.3.4.2)");
    }
}

TEST_CASE("Calling decode_spline_geometry() with something other than MoveTo command") {
    const container g = {vtzero::detail::command_line_to(3)};
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "expected command 1 but got 2");
    }
}

TEST_CASE("Calling decode_spline_geometry() with a count of 0") {
    const container g = {vtzero::detail::command_move_to(0)};
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "MoveTo command count is not 1 (spec 4.3.4.3)");
    }
}

TEST_CASE("Calling decode_spline_geometry() with a count of 2") {
    const container g = {vtzero::detail::command_move_to(2), 10, 20, 20, 10};
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "MoveTo command count is not 1 (spec 4.3.4.3)");
    }
}

TEST_CASE("Calling decode_spline_geometry() with 2nd command not a LineTo") {
    const container g = {vtzero::detail::command_move_to(1), 3, 4,
                         vtzero::detail::command_move_to(1)};
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "expected command 2 but got 1");
    }
}

TEST_CASE("Calling decode_spline_geometry() with LineTo and 0 count") {
    const container g = {vtzero::detail::command_move_to(1), 3, 4,
                         vtzero::detail::command_line_to(0)};
    const knots_container k = {1.0, 1.0, 1.0, 1.0};
    vtzero::detail::geometry_decoder<iterator, knots_iterator, 2> decoder{g.begin(), g.end(), k.begin(), k.end(), g.size() / 2};

    SECTION("check exception type") {
        REQUIRE_THROWS_AS(decoder.decode_spline(dummy_geom_handler{}),
                        vtzero::geometry_exception);
    }
    SECTION("check exception message") {
        REQUIRE_THROWS_WITH(decoder.decode_spline(dummy_geom_handler{}),
                            "LineTo command count is zero (spec 4.3.4.3)");
    }
}
