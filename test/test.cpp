#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "circbuf.h"

namespace
{

struct MoveOnly
{
    MoveOnly() = default;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly&
    operator=(MoveOnly&&) = default;
};

struct CopyOnly
{
    CopyOnly() = default;
    CopyOnly(const CopyOnly&) = default;
    CopyOnly&
    operator=(const CopyOnly&) = default;
};

struct NoDefaultConstructor
{
    NoDefaultConstructor(int, double)
    {
    }
};

} // namespace

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

TEST_CASE("test_comparison")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    auto cb2 = cb;
    REQUIRE(cb == cb2);
    cb2.push_back(44);
    REQUIRE(cb != cb2);
}

TEST_CASE("test_iterator_deref")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    // begin
    REQUIRE(*cb.begin() == 42);
    REQUIRE(*std::as_const(cb).begin() == 42);
    REQUIRE(cb.begin().operator->() == 42);
    REQUIRE(std::as_const(cb).begin().operator->() == 42);
    REQUIRE(cb.begin()[0] == 42);
    REQUIRE(std::as_const(cb).begin()[0] == 42);
    REQUIRE(cb.begin()[2] == 44);
    REQUIRE(std::as_const(cb).begin()[2] == 44);
    // end
    REQUIRE(*(--cb.end()) == 44);
    REQUIRE(*(--std::as_const(cb).end()) == 44);
    REQUIRE((--cb.end()).operator->() == 44);
    REQUIRE((--std::as_const(cb).end()).operator->() == 44);
    REQUIRE(cb.end()[-1] == 44);
    REQUIRE(std::as_const(cb).end()[-1] == 44);
    REQUIRE(cb.end()[-3] == 42);
    REQUIRE(std::as_const(cb).end()[-3] == 42);
    // distance
    REQUIRE(cb.size() ==
            static_cast<std::size_t>(std::distance(cb.begin(), cb.end())));
    REQUIRE(cb.size() ==
            static_cast<std::size_t>(std::distance(std::as_const(cb).begin(),
                                                   std::as_const(cb).end())));
}

TEST_CASE("test_iterator_increment")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.begin();
    REQUIRE(43 == *(++it));
    REQUIRE(43 == *it);
    REQUIRE(43 == *(it++));
    REQUIRE(46 == *(it + 2));
    auto it2 = cb.begin();
    it2 += 3;
    REQUIRE(45 == *it2);
    auto it3 = std::as_const(cb).begin() + 1;
    REQUIRE(4 == it2 + it3);
}

TEST_CASE("test_iterator_decrement")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.end();
    --it;
    REQUIRE(45 == *(--it));
    REQUIRE(45 == *it);
    REQUIRE(45 == *(it--));
    REQUIRE(42 == *(it - 2));
    auto it2 = cb.end();
    it2 -= 3;
    REQUIRE(44 == *it2);
    auto it3 = std::as_const(cb).end() - 1;
    REQUIRE(2 == it3 - it2);
}

TEST_CASE("test_iterator_comparison")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.begin();
    auto it2 = std::as_const(cb).begin();
    REQUIRE(it == it2);
    ++it;
    ++it2;
    REQUIRE(it == it2);
    REQUIRE(it <= it2);
    REQUIRE(it2 >= it);
    ++it2;
    REQUIRE(it != it2);
    REQUIRE(it < it2);
    REQUIRE(it2 > it);
}

TEST_CASE("test_global_begin_end")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    auto it = std::begin(cb);
    auto it2 = std::end(cb) - 1;
    REQUIRE(42 == *it);
    REQUIRE(44 == *it2);
}

TEST_CASE("test_for_loop")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb[1] = 60;
    std::vector<int> values;
    for (auto value : cb)
    {
        values.emplace_back(value);
    }
    const std::vector<int> exp{43, 60, 45};
    REQUIRE(exp == values);
}

TEST_CASE("test_with_move_only")
{
    using Buf = circbuf::CircularBuffer<MoveOnly, 3>;
    Buf cb;
    cb.push_back(MoveOnly{});
    cb.pop_front();
}

TEST_CASE("test_with_copy_only")
{
    using Buf = circbuf::CircularBuffer<CopyOnly, 3>;
    Buf cb;
    cb.push_back(CopyOnly{});
    cb.pop_front();
}

TEST_CASE("test_with_no_default_constructor")
{
    using Buf = circbuf::CircularBuffer<NoDefaultConstructor, 3>;
    Buf cb;
    cb.push_back(NoDefaultConstructor{42, 55.});
    cb.push_back(42, 55.);
    cb.pop_front();
}
