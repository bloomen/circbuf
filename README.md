circbuf
-------

[![Actions](https://github.com/bloomen/circbuf/actions/workflows/circbuf-tests.yml/badge.svg?branch=main)](https://github.com/bloomen/circbuf/actions/workflows/circbuf-tests.yml?query=branch%3Amain)

A simple circular buffer with a capacity fixed at compile time.
Requires C++20 and is tested with Clang, GCC, and Visual Studio.

Sample usage:
```cpp
CircularBuffer<int, 3> cb;
cb.push_back(42);
cb.push_back(43);
cb.push_back(44);
cb.push_back(45);
for (auto value : cb)
{
    std::cout << value << '\t';
}
// prints: 43, 44, 45
```
