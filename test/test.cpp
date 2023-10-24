#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "circbuf.h"

TEST_CASE("test_roundtrip")
{
    using Buf = circbuf::CircularBuffer<int, 2>;
    Buf cb;
    REQUIRE(2 == cb.capacity());
    REQUIRE(0 == cb.size());
    REQUIRE(cb.empty());
    REQUIRE(!cb.full());
    cb.push_back(42);
    REQUIRE(42 == cb.front());
    REQUIRE(42 == std::as_const(cb).front());
    REQUIRE(42 == cb.back());
    REQUIRE(42 == std::as_const(cb).back());
    REQUIRE(1 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(42 == cb[0]);
    REQUIRE(42 == std::as_const(cb)[0]);
    cb.push_back(43);
    REQUIRE(42 == cb.front());
    REQUIRE(42 == std::as_const(cb).front());
    REQUIRE(43 == cb.back());
    REQUIRE(43 == std::as_const(cb).back());
    REQUIRE(2 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    REQUIRE(42 == cb[0]);
    REQUIRE(42 == std::as_const(cb)[0]);
    REQUIRE(43 == cb[1]);
    REQUIRE(43 == std::as_const(cb)[1]);
    cb.push_back(44);
    REQUIRE(43 == cb.front());
    REQUIRE(43 == std::as_const(cb).front());
    REQUIRE(44 == cb.back());
    REQUIRE(44 == std::as_const(cb).back());
    REQUIRE(2 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    REQUIRE(43 == cb[0]);
    REQUIRE(43 == std::as_const(cb)[0]);
    REQUIRE(44 == cb[1]);
    REQUIRE(44 == std::as_const(cb)[1]);
    REQUIRE(43 == cb.pop_front());
    REQUIRE(44 == cb.front());
    REQUIRE(44 == std::as_const(cb).front());
    REQUIRE(44 == cb.back());
    REQUIRE(44 == std::as_const(cb).back());
    REQUIRE(1 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(44 == cb[0]);
    REQUIRE(44 == std::as_const(cb)[0]);
    REQUIRE(44 == cb.pop_front());
    REQUIRE(0 == cb.size());
    REQUIRE(cb.empty());
    REQUIRE(!cb.full());
}

TEST_CASE("test_object_creation")
{
    using Buf = circbuf::CircularBuffer<int, 2>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    // copy constructor
    Buf cb2{std::as_const(cb)};
    REQUIRE(cb2 == cb);
    // copy assignment
    Buf cb3;
    cb3 = std::as_const(cb);
    REQUIRE(cb3 == cb);
    // move constructor
    Buf cb4{std::move(cb)};
    REQUIRE(cb4 == cb);
    // move assignment
    Buf cb5;
    cb5 = std::move(cb2);
    REQUIRE(cb5 == cb2);
}
