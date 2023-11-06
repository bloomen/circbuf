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

    ~CircularBuffer()
    {
        destruct();
    }

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
            destruct();
            m_data = other.m_data;
            m_size = other.m_size;
            m_head = other.m_head;
            m_tail = other.m_tail;
        }
        return *this;
    }

    constexpr CircularBuffer(CircularBuffer&& other)
        : m_data{std::move(other.m_data)}
    {
        std::swap(m_size, other.m_size);
        std::swap(m_head, other.m_head);
        std::swap(m_tail, other.m_tail);
    }

    constexpr CircularBuffer&
    operator=(CircularBuffer&& other)
    {
        if (this != &other)
        {
            clear();
            m_data = std::move(other.m_data);
            std::swap(m_size, other.m_size);
            std::swap(m_head, other.m_head);
            std::swap(m_tail, other.m_tail);
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
        destruct();
        m_size = 0;
        m_head = 0;
        m_tail = 0;
    }

    constexpr reference
    operator[](const size_type index) noexcept
    {
        return at((m_head + index) % Capacity);
    }

    constexpr const_reference
    operator[](const size_type index) const noexcept
    {
        return at((m_head + index) % Capacity);
    }

    constexpr reference
    front() noexcept
    {
        return at(m_head);
    }

    constexpr const_reference
    front() const noexcept
    {
        return at(m_head);
    }

    constexpr reference
    back() noexcept
    {
        return at(m_tail);
    }

    constexpr const_reference
    back() const noexcept
    {
        return at(m_tail);
    }

    template <typename... Type>
    constexpr void
    push_back(Type&&... value)
    {
        if (empty())
        {
            ++m_size;
        }
        else if (full())
        {
            m_head = (m_head + 1) % Capacity;
            m_tail = (m_tail + 1) % Capacity;
        }
        else
        {
            ++m_tail;
            ++m_size;
        }
        new (m_data[m_tail]) value_type{std::forward<Type>(value)...};
    }

    constexpr value_type
    pop_front()
    {
        const auto index = m_head;
        m_head = (m_head + 1) % Capacity;
        --m_size;
        auto value = std::move(at(index));
        at(index).~value_type();
        return value;
    }

    constexpr iterator
    begin()
    {
        return iterator{*this, 0};
    }

    constexpr const_iterator
    begin() const
    {
        return const_iterator{*this, 0};
    }

    constexpr iterator
    end()
    {
        return iterator{*this, m_size};
    }

    constexpr const_iterator
    end() const
    {
        return const_iterator{*this, m_size};
    }

private:
    template <typename BufferType>
    friend class CircularBufferIterator;

    value_type&
    at(const size_type index)
    {
        return *reinterpret_cast<value_type*>(&m_data[index]);
    }

    const value_type&
    at(const size_type index) const
    {
        return *reinterpret_cast<const value_type*>(&m_data[index]);
    }

    constexpr void
    destruct() noexcept
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            at(i).~value_type();
        }
    }

    using Memory = unsigned char[sizeof(value_type)];
    std::array<Memory, Capacity> m_data;
    size_type m_size{};
    size_type m_head{};
    size_type m_tail{};
};

template <typename T, std::size_t Capacity>
constexpr bool
operator==(const CircularBuffer<T, Capacity>& lhs,
           const CircularBuffer<T, Capacity>& rhs) noexcept
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

template <typename T, std::size_t Capacity>
constexpr bool
operator!=(const CircularBuffer<T, Capacity>& lhs,
           const CircularBuffer<T, Capacity>& rhs) noexcept
{
    return !(lhs == rhs);
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

    explicit constexpr CircularBufferIterator(BufferType& buffer,
                                              const size_type index)
        : m_buffer{buffer}
        , m_index{index}
    {
    }

    constexpr reference
    operator*() noexcept
        requires(!std::is_const_v<BufferType>)
    {
        return m_buffer.get().at((m_buffer.get().m_head + m_index) %
                                 BufferType::capacity());
    }

    constexpr const_reference
    operator*() const noexcept
    {
        return m_buffer.get().at((m_buffer.get().m_head + m_index) %
                                 BufferType::capacity());
    }

    constexpr reference
    operator->() noexcept
        requires(!std::is_const_v<BufferType>)
    {
        return this->operator*();
    }

    constexpr const_reference
    operator->() const noexcept
    {
        return this->operator*();
    }

    constexpr self_type&
    operator++() noexcept
    {
        ++m_index;
        return *this;
    }

    constexpr self_type
    operator++(int)
    {
        self_type temp = *this;
        ++*this;
        return temp;
    }

    constexpr self_type
    operator+(const difference_type offset) const
    {
        self_type temp = *this;
        return temp += offset;
    }

    constexpr self_type&
    operator+=(const difference_type offset)
    {
        const difference_type next =
            (m_index + offset) % BufferType::capacity();
        m_index = next;
        return *this;
    }

    constexpr self_type&
    operator--() noexcept
    {
        --m_index;
        return *this;
    }

    constexpr self_type
    operator--(int)
    {
        self_type temp = *this;
        --*this;
        return temp;
    }

    constexpr self_type
    operator-(const difference_type offset) const
    {
        self_type temp = *this;
        return temp -= offset;
    }

    constexpr self_type&
    operator-=(const difference_type offset) noexcept
    {
        return *this += -offset;
    }

    value_type&
    operator[](const difference_type offset) noexcept
        requires(!std::is_const_v<BufferType>)
    {
        return *(*this + offset);
    }

    const value_type&
    operator[](const difference_type offset) const noexcept
    {
        return *(*this + offset);
    }

private:
    template <typename BufferType1, typename BufferType2>
    friend constexpr typename BufferType1::difference_type
    operator+(const CircularBufferIterator<BufferType1>&,
              const CircularBufferIterator<BufferType2>&) noexcept;

    template <typename BufferType1, typename BufferType2>
    friend constexpr typename BufferType1::difference_type
    operator-(const CircularBufferIterator<BufferType1>&,
              const CircularBufferIterator<BufferType2>&) noexcept;

    template <typename BufferType1, typename BufferType2>
    friend constexpr bool
    operator==(const CircularBufferIterator<BufferType1>&,
               const CircularBufferIterator<BufferType2>&) noexcept;

    template <typename BufferType1, typename BufferType2>
    friend constexpr auto
    operator<=>(const CircularBufferIterator<BufferType1>&,
                const CircularBufferIterator<BufferType2>&) noexcept;

    std::reference_wrapper<BufferType> m_buffer;
    size_type m_index;
};

template <typename BufferType1, typename BufferType2>
constexpr typename BufferType1::difference_type
operator+(const CircularBufferIterator<BufferType1>& lhs,
          const CircularBufferIterator<BufferType2>& rhs) noexcept
{
    return lhs.m_index + rhs.m_index;
}

template <typename BufferType1, typename BufferType2>
constexpr typename BufferType1::difference_type
operator-(const CircularBufferIterator<BufferType1>& lhs,
          const CircularBufferIterator<BufferType2>& rhs) noexcept
{
    return lhs.m_index - rhs.m_index;
}

template <typename BufferType1, typename BufferType2>
constexpr bool
operator==(const CircularBufferIterator<BufferType1>& lhs,
           const CircularBufferIterator<BufferType2>& rhs) noexcept
{
    return lhs.m_index == rhs.m_index;
}

template <typename BufferType1, typename BufferType2>
constexpr auto
operator<=>(const CircularBufferIterator<BufferType1>& lhs,
            const CircularBufferIterator<BufferType2>& rhs) noexcept
{
    return lhs.m_index <=> rhs.m_index;
}

} // namespace circbuf
