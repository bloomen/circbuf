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
    REQUIRE(42 == const_cast<const Buf&>(cb).front());
    REQUIRE(42 == cb.back());
    REQUIRE(42 == const_cast<const Buf&>(cb).back());
    REQUIRE(1 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(42 == cb[0]);
    REQUIRE(42 == const_cast<const Buf&>(cb)[0]);
    cb.push_back(43);
    REQUIRE(42 == cb.front());
    REQUIRE(42 == const_cast<const Buf&>(cb).front());
    REQUIRE(43 == cb.back());
    REQUIRE(43 == const_cast<const Buf&>(cb).back());
    REQUIRE(2 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    REQUIRE(42 == cb[0]);
    REQUIRE(42 == const_cast<const Buf&>(cb)[0]);
    REQUIRE(43 == cb[1]);
    REQUIRE(43 == const_cast<const Buf&>(cb)[1]);
    cb.push_back(44);
    REQUIRE(43 == cb.front());
    REQUIRE(43 == const_cast<const Buf&>(cb).front());
    REQUIRE(44 == cb.back());
    REQUIRE(44 == const_cast<const Buf&>(cb).back());
    REQUIRE(2 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    REQUIRE(43 == cb[0]);
    REQUIRE(43 == const_cast<const Buf&>(cb)[0]);
    REQUIRE(44 == cb[1]);
    REQUIRE(44 == const_cast<const Buf&>(cb)[1]);
    REQUIRE(43 == cb.pop_front());
    REQUIRE(44 == cb.front());
    REQUIRE(44 == const_cast<const Buf&>(cb).front());
    REQUIRE(44 == cb.back());
    REQUIRE(44 == const_cast<const Buf&>(cb).back());
    REQUIRE(1 == cb.size());
    REQUIRE(!cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(44 == cb[0]);
    REQUIRE(44 == const_cast<const Buf&>(cb)[0]);
    REQUIRE(44 == cb.pop_front());
    REQUIRE(0 == cb.size());
    REQUIRE(cb.empty());
    REQUIRE(!cb.full());
}
