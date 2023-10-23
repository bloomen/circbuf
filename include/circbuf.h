#include <array>
#include <cstddef>
#include <type_traits>

namespace circbuf
{

template <typename T, std::size_t N>
    requires(N > 0)
class CircularBufferIterator;

template <typename T, std::size_t N>
    requires(N > 0)
class CircularBuffer
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = CircularBufferIterator<T, N>;
    using const_iterator = CircularBufferIterator<const T, N>;

    constexpr CircularBuffer() = default;

    constexpr CircularBuffer(const CircularBuffer& other)
        : m_data{other.m_data}
        , m_size{other.m_size}
        , m_head{other.m_head}
        , m_tail{other.m_tail}
    {
    }

    constexpr CircularBuffer&
    operator=(const CircularBuffer& other)
    {
        if (this != &other)
        {
            m_data = other.m_data;
            m_size = other.m_size;
            m_head = other.m_head;
            m_tail = other.m_tail;
        }
        return this;
    }

    constexpr CircularBuffer(CircularBuffer&& other)
        : m_data{std::move(other.m_data)}
        , m_size{other.m_size}
        , m_head{other.m_head}
        , m_tail{other.m_tail}
    {
    }

    constexpr CircularBuffer&
    operator=(CircularBuffer&& other)
    {
        if (this != &other)
        {
            m_data = std::move(other.m_data);
            m_size = other.m_size;
            m_head = other.m_head;
            m_tail = other.m_tail;
        }
        return this;
    }

    constexpr size_type
    capacity() const noexcept
    {
        return N;
    }

    constexpr size_type
    size() const noexcept
    {
        return m_size;
    }

    constexpr bool
    empty() const noexcept
    {
        return m_size == 0;
    }

    constexpr bool
    full() const noexcept
    {
        return m_size == N;
    }

    constexpr void
    clear() noexcept
    {
        m_size = m_head = m_head = 0;
    }

    constexpr reference
    operator[](const size_type index) noexcept
    {
        return m_data[(m_head + index) % N];
    }

    constexpr const_reference
    operator[](const size_type index) const noexcept
    {
        return m_data[(m_head + index) % N];
    }

    constexpr reference
    front() noexcept
    {
        return m_data[m_head];
    }

    constexpr const_reference
    front() const noexcept
    {
        return m_data[m_head];
    }

    constexpr reference
    back() noexcept
    {
        return m_data[m_tail];
    }

    constexpr const_reference
    back() const noexcept
    {
        return m_data[m_tail];
    }

    template <typename Type>
        requires(std::is_same_v<std::decay_t<Type>, value_type>)
    constexpr void
    push_back(Type&& value)
    {
        if (empty())
        {
            m_data[m_tail] = std::forward<Type>(value);
            ++m_size;
        }
        else if (!full())
        {
            m_data[++m_tail] = std::forward<Type>(value);
            ++m_size;
        }
        else
        {
            m_head = (m_head + 1) % N;
            m_tail = (m_tail + 1) % N;
            m_data[m_tail] = std::forward<Type>(value);
        }
    }

    constexpr value_type
    pop_front()
    {
        const auto index = m_head;
        m_head = (m_head + 1) % N;
        --m_size;
        return m_data[index];
    }

private:
    std::array<value_type, N> m_data;
    size_type m_size{};
    size_type m_head{};
    size_type m_tail{};
};

} // namespace circbuf
