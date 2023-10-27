#include <array>
#include <cstddef>
#include <type_traits>

namespace circbuf
{

template <typename BufferType>
class CircularBufferIterator;

template <typename T, std::size_t Capacity>
    requires(Capacity > 0)
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
    using iterator = CircularBufferIterator<CircularBuffer>;
    using const_iterator = CircularBufferIterator<const CircularBuffer>;

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
        return *this;
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
        return *this;
    }

    constexpr static size_type
    capacity() noexcept
    {
        return Capacity;
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
        return m_size == Capacity;
    }

    constexpr void
    clear() noexcept
    {
        m_size = m_head = m_head = 0;
    }

    constexpr reference
    operator[](const size_type index) noexcept
    {
        return m_data[(m_head + index) % Capacity];
    }

    constexpr const_reference
    operator[](const size_type index) const noexcept
    {
        return m_data[(m_head + index) % Capacity];
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
            m_head = (m_head + 1) % Capacity;
            m_tail = (m_tail + 1) % Capacity;
            m_data[m_tail] = std::forward<Type>(value);
        }
    }

    constexpr value_type
    pop_front()
    {
        const auto index = m_head;
        m_head = (m_head + 1) % Capacity;
        --m_size;
        return m_data[index];
    }

    iterator
    begin()
    {
        return iterator{*this, 0};
    }

    const_iterator
    begin() const
    {
        return const_iterator{*this, 0};
    }

    iterator
    end()
    {
        return iterator{*this, m_size};
    }

    const_iterator
    end() const
    {
        return const_iterator{*this, m_size};
    }

private:
    template <typename BufferType_>
    friend class CircularBufferIterator;

    template <typename T_, std::size_t Capacity_>
    friend constexpr bool
    operator==(const CircularBuffer<T_, Capacity_>&,
               const CircularBuffer<T_, Capacity_>&);

    std::array<value_type, Capacity> m_data;
    size_type m_size{};
    size_type m_head{};
    size_type m_tail{};
};

template <typename T, std::size_t Capacity>
constexpr bool
operator==(const CircularBuffer<T, Capacity>& lhs,
           const CircularBuffer<T, Capacity>& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    for (typename CircularBuffer<T, Capacity>::size_type i = 0; i < lhs.size();
         ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <typename BufferType>
class CircularBufferIterator
{
public:
    using self_type = CircularBufferIterator;
    using value_type = typename BufferType::value_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    explicit CircularBufferIterator(BufferType& buffer, const size_type index)
        : m_buffer{buffer}
        , m_index{index}
    {
    }

    reference
    operator*()
        requires(!std::is_const_v<BufferType>)
    {
        return m_buffer.get()
            .m_data[(m_buffer.get().m_head + m_index) % BufferType::capacity()];
    }

    const_reference
    operator*() const
    {
        return m_buffer.get()
            .m_data[(m_buffer.get().m_head + m_index) % BufferType::capacity()];
    }

    reference
    operator->()
        requires(!std::is_const_v<BufferType>)
    {
        return this->operator*();
    }

    const_reference
    operator->() const
    {
        return this->operator*();
    }

    self_type&
    operator++()
    {
        ++m_index;
        return *this;
    }

    self_type
    operator++(int)
    {
        self_type temp = *this;
        ++*this;
        return temp;
    }

    self_type&
    operator--()
    {
        --m_index;
        return *this;
    }

    self_type
    operator--(int)
    {
        self_type temp = *this;
        --*this;
        return temp;
    }

    self_type
    operator+(const difference_type offset) const
    {
        self_type temp = *this;
        return temp += offset;
    }

    self_type
    operator-(const difference_type offset) const
    {
        self_type temp = *this;
        return temp -= offset;
    }

    difference_type
    operator-(const self_type& other) const
    {
        return m_index - other.m_index;
    }

    difference_type
    operator+(const self_type& other) const
    {
        return m_index + other.m_index;
    }

    self_type&
    operator+=(const difference_type offset)
    {
        const difference_type next =
            (m_index + offset) % BufferType::capacity();
        m_index = next;
        return *this;
    }

    self_type&
    operator-=(const difference_type offset)
    {
        return *this += -offset;
    }

    bool
    operator==(const self_type& other) const
    {
        return m_index == other.m_index;
    }

    bool
    operator!=(const self_type& other) const
    {
        return !(*this == other);
    }

    bool
    operator<(const self_type& other) const
    {
        return m_index < other.m_index;
    }

    bool
    operator>(const self_type& other) const
    {
        return other < *this;
    }

    bool
    operator<=(const self_type& other) const
    {
        return !(other < *this);
    }

    bool
    operator>=(const self_type& other) const
    {
        return !(*this < other);
    }

    value_type&
    operator[](const difference_type offset)
    {
        return *(*this + offset);
    }

    const value_type&
    operator[](const difference_type offset) const
    {
        return *(*this + offset);
    }

private:
    std::reference_wrapper<BufferType> m_buffer;
    size_type m_index;
};

} // namespace circbuf
