#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "circbuf.h"

#ifndef __APPLE__ // no ranges support on Apple platform
#include <ranges>
#endif

static_assert(std::is_same<std::random_access_iterator_tag,
                           typename std::iterator_traits<
                               circbuf::CircularBuffer<int, 3>::iterator>::
                               iterator_category>::value,
              "iterator is random access");

static_assert(
    std::is_same<std::random_access_iterator_tag,
                 typename std::iterator_traits<
                     circbuf::CircularBuffer<int, 3>::const_iterator>::
                     iterator_category>::value,
    "const iterator is random access");

static_assert(
    std::is_same<std::random_access_iterator_tag,
                 typename std::iterator_traits<
                     circbuf::CircularBuffer<int, 3>::reverse_iterator>::
                     iterator_category>::value,
    "reverse iterator is random access");

static_assert(
    std::is_same<std::random_access_iterator_tag,
                 typename std::iterator_traits<
                     circbuf::CircularBuffer<int, 3>::const_reverse_iterator>::
                     iterator_category>::value,
    "const reverse iterator is random access");

#ifndef __APPLE__ // no ranges support on Apple platform
static_assert(std::ranges::random_access_range<circbuf::CircularBuffer<int, 3>>,
              "buffer is random access range");

static_assert(
    std::ranges::random_access_range<const circbuf::CircularBuffer<int, 3>>,
    "const buffer is random access range");
#endif

namespace
{

struct MoveOnly
{
    MoveOnly()
    {
        data = new int[3]{};
    }
    ~MoveOnly()
    {
        delete[] data;
    }
    MoveOnly(MoveOnly&& o)
    {
        std::swap(data, o.data);
    }
    MoveOnly&
    operator=(MoveOnly&& o)
    {
        if (this != &o)
        {
            delete[] data;
            data = nullptr;
            std::swap(data, o.data);
        }
        return *this;
    }
    int* data = nullptr;
};

struct CopyOnly
{
    CopyOnly()
    {
        data = new int[3]{};
    }
    ~CopyOnly()
    {
        delete[] data;
    }
    CopyOnly(const CopyOnly& o)
    {
        data = new int[3];
        std::copy(o.data, o.data + 3, data);
    }
    CopyOnly&
    operator=(const CopyOnly& o)
    {
        if (this != &o)
        {
            delete[] data;
            data = new int[3];
            std::copy(o.data, o.data + 3, data);
        }
        return *this;
    }
    int* data = nullptr;
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
    REQUIRE(2 == cb.max_size());
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

TEST_CASE("test_with_capacity_of_one")
{
    using Buf = circbuf::CircularBuffer<int, 1>;
    Buf cb;
    cb.push_back(42);
    REQUIRE(42 == cb.front());
    REQUIRE(42 == cb.back());
    REQUIRE(42 == *cb.begin());
    cb.push_back(43);
    REQUIRE(43 == cb.front());
    REQUIRE(43 == cb.back());
    REQUIRE(43 == *cb.begin());
    REQUIRE(43 == cb.pop_front());
    REQUIRE(cb.empty());
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
    REQUIRE(cb4 == cb3);
    REQUIRE(cb.empty());
    // move assignment
    Buf cb5;
    cb5 = std::move(cb2);
    REQUIRE(cb5 == cb3);
    REQUIRE(cb2.empty());
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

TEST_CASE("test_smaller")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    auto cb2 = cb;
    REQUIRE(cb <= cb2);
    cb2[1] = 44;
    REQUIRE(cb < cb2);
    REQUIRE(cb <= cb2);
    cb2[1] = 43;
    cb.push_back(45);
    REQUIRE(cb2 < cb);
    REQUIRE(cb2 <= cb);
}

TEST_CASE("test_greater")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    auto cb2 = cb;
    REQUIRE(cb >= cb2);
    cb2[1] = 41;
    REQUIRE(cb > cb2);
    REQUIRE(cb >= cb2);
    cb2[1] = 43;
    cb2.push_back(45);
    REQUIRE(cb2 > cb);
    REQUIRE(cb2 >= cb);
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

TEST_CASE("test_reverse_iterator_deref")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    // begin
    REQUIRE(*cb.rbegin() == 44);
    REQUIRE(*std::as_const(cb).rbegin() == 44);
    REQUIRE(cb.rbegin().operator->() == 44);
    REQUIRE(std::as_const(cb).rbegin().operator->() == 44);
    REQUIRE(cb.rbegin()[0] == 44);
    REQUIRE(std::as_const(cb).rbegin()[0] == 44);
    REQUIRE(cb.rbegin()[2] == 42);
    REQUIRE(std::as_const(cb).rbegin()[2] == 42);
    // end
    REQUIRE(*(--cb.rend()) == 42);
    REQUIRE(*(--std::as_const(cb).rend()) == 42);
    REQUIRE((--cb.rend()).operator->() == 42);
    REQUIRE((--std::as_const(cb).rend()).operator->() == 42);
    REQUIRE(cb.rend()[-1] == 42);
    REQUIRE(std::as_const(cb).rend()[-1] == 42);
    REQUIRE(cb.rend()[-3] == 44);
    REQUIRE(std::as_const(cb).rend()[-3] == 44);
    // distance
    REQUIRE(cb.size() ==
            static_cast<std::size_t>(std::distance(cb.rbegin(), cb.rend())));
    REQUIRE(cb.size() ==
            static_cast<std::size_t>(std::distance(std::as_const(cb).rbegin(),
                                                   std::as_const(cb).rend())));
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
    auto it4 = 2 + it3;
    REQUIRE(45 == *it4);
}

TEST_CASE("test_reverse_iterator_increment")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.rbegin();
    REQUIRE(45 == *(++it));
    REQUIRE(45 == *it);
    REQUIRE(45 == *(it++));
    REQUIRE(42 == *(it + 2));
    auto it2 = cb.rbegin();
    it2 += 3;
    REQUIRE(43 == *it2);
    auto it3 = std::as_const(cb).rbegin() + 1;
    REQUIRE(4 == it2 + it3);
    auto it4 = 2 + it3;
    REQUIRE(43 == *it4);
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
    auto it4 = 7 - it3;
    REQUIRE(45 == *it4);
}

TEST_CASE("test_reverse_iterator_decrement")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.rend();
    --it;
    REQUIRE(43 == *(--it));
    REQUIRE(43 == *it);
    REQUIRE(43 == *(it--));
    REQUIRE(46 == *(it - 2));
    auto it2 = cb.rend();
    it2 -= 3;
    REQUIRE(44 == *it2);
    auto it3 = std::as_const(cb).rend() - 1;
    REQUIRE(2 == it3 - it2);
    auto it4 = 7 - it3;
    REQUIRE(43 == *it4);
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

TEST_CASE("test_reverse_iterator_comparison")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    cb.push_back(45);
    cb.push_back(46);
    auto it = cb.rbegin();
    auto it2 = std::as_const(cb).rbegin();
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
    REQUIRE(std::begin(cb) == std::cbegin(cb));
    REQUIRE(std::end(cb) == std::cend(cb));
}

TEST_CASE("test_global_rbegin_rend")
{
    using Buf = circbuf::CircularBuffer<int, 5>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    auto it = std::rbegin(cb);
    auto it2 = std::rend(cb) - 1;
    REQUIRE(44 == *it);
    REQUIRE(42 == *it2);
    REQUIRE(std::rbegin(cb) == std::crbegin(cb));
    REQUIRE(std::rend(cb) == std::crend(cb));
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

TEST_CASE("test_cleanup")
{
    using Buf = circbuf::CircularBuffer<MoveOnly, 3>;
    Buf cb;
    cb.push_back(MoveOnly{});
}

TEST_CASE("test_const_iterator_methods")
{
    using Buf = circbuf::CircularBuffer<CopyOnly, 3>;
    Buf cb;
    cb.push_back(CopyOnly{});
    REQUIRE(std::as_const(cb).begin() == cb.cbegin());
    REQUIRE(std::as_const(cb).end() == cb.cend());
    REQUIRE(std::as_const(cb).rbegin() == cb.crbegin());
    REQUIRE(std::as_const(cb).rend() == cb.crend());
}

TEST_CASE("test_iterator_copy")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    auto it = cb.begin();
    ++it;
    auto it2 = it;
    REQUIRE(*it == *it2);
    decltype(it) it3;
    it3 = it;
    REQUIRE(*it == *it3);
}

#ifndef __APPLE__ // no ranges support on Apple platform
TEST_CASE("test_ranges_begin_end")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(42);
    cb.push_back(43);
    cb.push_back(44);
    auto begin = std::ranges::begin(cb);
    auto end = std::ranges::end(cb);
    REQUIRE(begin != end);
}

TEST_CASE("test_ranges_sort")
{
    using Buf = circbuf::CircularBuffer<int, 3>;
    Buf cb;
    cb.push_back(44);
    cb.push_back(42);
    cb.push_back(43);
    std::ranges::sort(cb);
    Buf cb2;
    cb2.push_back(42);
    cb2.push_back(43);
    cb2.push_back(44);
    REQUIRE(cb == cb2);
}
#endif
