#include <array>
#include <cstddef>
#include <type_traits>
#include <variant>

namespace circbuf
{

template <typename BufferType, bool Reverse>
class CircularBufferIterator;

template <typename T, std::size_t MaxSize>
    requires(MaxSize > 0)
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
    using iterator = CircularBufferIterator<CircularBuffer, false>;
    using const_iterator = CircularBufferIterator<const CircularBuffer, false>;
    using reverse_iterator = CircularBufferIterator<CircularBuffer, true>;
    using const_reverse_iterator =
        CircularBufferIterator<const CircularBuffer, true>;

    constexpr CircularBuffer() = default;

    constexpr ~CircularBuffer() noexcept(
        std::is_nothrow_destructible_v<value_type>) = default;

    constexpr CircularBuffer(const CircularBuffer& other) noexcept(
        std::is_nothrow_copy_constructible_v<value_type>)
    {
        copy_from(other);
    }

    constexpr CircularBuffer&
    operator=(const CircularBuffer& other) noexcept(
        std::is_nothrow_destructible_v<value_type>&&
            std::is_nothrow_copy_constructible_v<value_type>)
    {
        if (this != &other)
        {
            clear();
            copy_from(other);
        }
        return *this;
    }

    constexpr CircularBuffer(CircularBuffer&& other) noexcept(
        std::is_nothrow_move_constructible_v<value_type>)
    {
        move_from(std::move(other));
    }

    constexpr CircularBuffer&
    operator=(CircularBuffer&& other) noexcept(
        std::is_nothrow_destructible_v<value_type>&&
            std::is_nothrow_move_constructible_v<value_type>)
    {
        if (this != &other)
        {
            clear();
            move_from(std::move(other));
        }
        return *this;
    }

    consteval static size_type
    max_size() noexcept
    {
        return MaxSize;
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
        return m_size == MaxSize;
    }

    constexpr void
    clear() noexcept(std::is_nothrow_destructible_v<value_type>)
    {
        std::destroy(m_data.begin(), m_data.end());
        m_size = 0;
        m_head = 0;
        m_tail = 0;
    }

    constexpr reference
    operator[](const size_type index) noexcept
    {
        return at((m_head + index) % MaxSize);
    }

    constexpr const_reference
    operator[](const size_type index) const noexcept
    {
        return at((m_head + index) % MaxSize);
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

    constexpr void
    push_back(const value_type& value) noexcept(
        std::is_nothrow_copy_constructible_v<value_type>)
    {
        increment();
        m_data[m_tail] = Memory{std::in_place_type_t<value_type>{}, value};
    }

    constexpr void
    push_back(value_type&& value) noexcept(
        std::is_nothrow_move_constructible_v<value_type>)
    {
        increment();
        m_data[m_tail] =
            Memory{std::in_place_type_t<value_type>{}, std::move(value)};
    }

    template <typename... Type>
    constexpr void
    emplace_back(Type&&... value) noexcept(
        std::is_nothrow_constructible_v<value_type>)
    {
        increment();
        m_data[m_tail] = Memory{std::in_place_type_t<value_type>{},
                                std::forward<Type>(value)...};
    }

    constexpr value_type
    pop_front() noexcept(
        std::is_nothrow_destructible_v<value_type>&&
            std::is_nothrow_move_constructible_v<value_type>&&
                std::is_nothrow_copy_constructible_v<value_type>)
    {
        const auto index = m_head;
        m_head = (m_head + 1) % MaxSize;
        --m_size;
        auto value = std::move(at(index));
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

    constexpr const_iterator
    cbegin() const
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

    constexpr const_iterator
    cend() const
    {
        return const_iterator{*this, m_size};
    }

    constexpr reverse_iterator
    rbegin()
    {
        return reverse_iterator{*this, 0};
    }

    constexpr const_reverse_iterator
    rbegin() const
    {
        return const_reverse_iterator{*this, 0};
    }

    constexpr const_reverse_iterator
    crbegin() const
    {
        return const_reverse_iterator{*this, 0};
    }

    constexpr reverse_iterator
    rend()
    {
        return reverse_iterator{*this, m_size};
    }

    constexpr const_reverse_iterator
    rend() const
    {
        return const_reverse_iterator{*this, m_size};
    }

    constexpr const_reverse_iterator
    crend() const
    {
        return const_reverse_iterator{*this, m_size};
    }

private:
    template <typename BufferType, bool Reverse>
    friend class CircularBufferIterator;

    constexpr value_type&
    at(const size_type index) noexcept
    {
        return std::get<value_type>(m_data[index]);
    }

    constexpr const value_type&
    at(const size_type index) const noexcept
    {
        return std::get<value_type>(m_data[index]);
    }

    constexpr void
    copy_from(const CircularBuffer& other) noexcept(
        std::is_nothrow_copy_constructible_v<value_type>)
    {
        for (const auto& value : other)
        {
            push_back(value);
        }
    }

    constexpr void
    move_from(CircularBuffer&& other) noexcept(
        std::is_nothrow_move_constructible_v<value_type>)
    {
        for (auto&& value : other)
        {
            push_back(std::move(value));
        }
        other.m_size = 0;
        other.m_head = 0;
        other.m_tail = 0;
    }

    constexpr void
    increment() noexcept
    {
        if (empty())
        {
            ++m_size;
        }
        else if (full())
        {
            m_head = (m_head + 1) % MaxSize;
            m_tail = (m_tail + 1) % MaxSize;
        }
        else
        {
            ++m_tail;
            ++m_size;
        }
    }

    using Memory = std::variant<std::monostate, value_type>;
    std::array<Memory, MaxSize> m_data;
    size_type m_size{};
    size_type m_head{};
    size_type m_tail{};
};

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator==(const CircularBuffer<T, MaxSize1>& lhs,
           const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator!=(const CircularBuffer<T, MaxSize1>& lhs,
           const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator<(const CircularBuffer<T, MaxSize1>& lhs,
          const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator>(const CircularBuffer<T, MaxSize1>& lhs,
          const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    return rhs < lhs;
}

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator<=(const CircularBuffer<T, MaxSize1>& lhs,
           const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    return !(lhs > rhs);
}

template <typename T, std::size_t MaxSize1, std::size_t MaxSize2>
constexpr bool
operator>=(const CircularBuffer<T, MaxSize1>& lhs,
           const CircularBuffer<T, MaxSize2>& rhs) noexcept
{
    return !(lhs < rhs);
}

template <typename BufferType, bool Reverse>
class CircularBufferIterator
{
public:
    using self_type = CircularBufferIterator;
    using value_type = typename BufferType::value_type;
    using size_type = typename BufferType::size_type;
    using difference_type = typename BufferType::difference_type;
    using reference = typename BufferType::reference;
    using const_reference = typename BufferType::const_reference;
    using pointer = typename BufferType::pointer;
    using const_pointer = typename BufferType::const_pointer;
    using iterator_category = std::random_access_iterator_tag;
    using contained_ref = std::
        conditional_t<std::is_const_v<BufferType>, const_reference, reference>;
    using contained_ptr =
        std::conditional_t<std::is_const_v<BufferType>, const_pointer, pointer>;

    explicit constexpr CircularBufferIterator() = default;

    explicit constexpr CircularBufferIterator(BufferType& buffer,
                                              const size_type index)
        : m_buffer{&buffer}
        , m_index{static_cast<difference_type>(index)}
    {
    }

    constexpr contained_ref
    operator*() noexcept
    {
        if constexpr (Reverse)
        {
            return m_buffer->at((m_buffer->m_head + m_buffer->m_size -
                                 static_cast<size_type>(m_index) - 1) %
                                BufferType::max_size());
        }
        else
        {
            return m_buffer->at(
                (m_buffer->m_head + static_cast<size_type>(m_index)) %
                BufferType::max_size());
        }
    }

    constexpr contained_ref
    operator*() const noexcept
    {
        if constexpr (Reverse)
        {
            return m_buffer->at((m_buffer->m_head + m_buffer->m_size -
                                 static_cast<size_type>(m_index) - 1) %
                                BufferType::max_size());
        }
        else
        {
            return m_buffer->at(
                (m_buffer->m_head + static_cast<size_type>(m_index)) %
                BufferType::max_size());
        }
    }

    constexpr contained_ptr
    operator->() noexcept
    {
        return &this->operator*();
    }

    constexpr contained_ptr
    operator->() const noexcept
    {
        return &this->operator*();
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
        const difference_type next = (m_index + offset) %
            static_cast<difference_type>(BufferType::max_size());
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

    constexpr contained_ref
    operator[](const difference_type offset) noexcept
    {
        return *(*this + offset);
    }

    constexpr contained_ref
    operator[](const difference_type offset) const noexcept
    {
        return *(*this + offset);
    }

private:
    template <typename BufferType1,
              bool Reverse1,
              typename BufferType2,
              bool Reverse2>
        requires(std::is_same_v<std::remove_const_t<BufferType1>,
                                std::remove_const_t<BufferType2>> &&
                 ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
    friend constexpr typename BufferType1::difference_type
    operator+(const CircularBufferIterator<BufferType1, Reverse1>&,
              const CircularBufferIterator<BufferType2, Reverse2>&) noexcept;

    template <typename BufferType1,
              bool Reverse1,
              typename BufferType2,
              bool Reverse2>
        requires(std::is_same_v<std::remove_const_t<BufferType1>,
                                std::remove_const_t<BufferType2>> &&
                 ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
    friend constexpr typename BufferType1::difference_type
    operator-(const CircularBufferIterator<BufferType1, Reverse1>&,
              const CircularBufferIterator<BufferType2, Reverse2>&) noexcept;

    template <typename BufferType1,
              bool Reverse1,
              typename BufferType2,
              bool Reverse2>
        requires(std::is_same_v<std::remove_const_t<BufferType1>,
                                std::remove_const_t<BufferType2>> &&
                 ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
    friend constexpr bool
    operator==(const CircularBufferIterator<BufferType1, Reverse1>&,
               const CircularBufferIterator<BufferType2, Reverse2>&) noexcept;

    template <typename BufferType1,
              bool Reverse1,
              typename BufferType2,
              bool Reverse2>
        requires(std::is_same_v<std::remove_const_t<BufferType1>,
                                std::remove_const_t<BufferType2>> &&
                 ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
    friend constexpr auto
    operator<=>(const CircularBufferIterator<BufferType1, Reverse1>&,
                const CircularBufferIterator<BufferType2, Reverse2>&) noexcept;

    template <typename BufferType1, bool Reverse1>
    friend constexpr auto
    operator-(const typename CircularBufferIterator<BufferType1,
                                                    Reverse1>::difference_type,
              const CircularBufferIterator<BufferType1, Reverse1>&) noexcept;

    BufferType* m_buffer{};
    difference_type m_index{};
};

template <typename BufferType1,
          bool Reverse1,
          typename BufferType2,
          bool Reverse2>
    requires(std::is_same_v<std::remove_const_t<BufferType1>,
                            std::remove_const_t<BufferType2>> &&
             ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
constexpr typename BufferType1::difference_type
operator+(const CircularBufferIterator<BufferType1, Reverse1>& lhs,
          const CircularBufferIterator<BufferType2, Reverse2>& rhs) noexcept
{
    return lhs.m_index + rhs.m_index;
}

template <typename BufferType1,
          bool Reverse1,
          typename BufferType2,
          bool Reverse2>
    requires(std::is_same_v<std::remove_const_t<BufferType1>,
                            std::remove_const_t<BufferType2>> &&
             ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
constexpr typename BufferType1::difference_type
operator-(const CircularBufferIterator<BufferType1, Reverse1>& lhs,
          const CircularBufferIterator<BufferType2, Reverse2>& rhs) noexcept
{
    return lhs.m_index - rhs.m_index;
}

template <typename BufferType1,
          bool Reverse1,
          typename BufferType2,
          bool Reverse2>
    requires(std::is_same_v<std::remove_const_t<BufferType1>,
                            std::remove_const_t<BufferType2>> &&
             ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
constexpr bool
operator==(const CircularBufferIterator<BufferType1, Reverse1>& lhs,
           const CircularBufferIterator<BufferType2, Reverse2>& rhs) noexcept
{
    return lhs.m_index == rhs.m_index;
}

template <typename BufferType1,
          bool Reverse1,
          typename BufferType2,
          bool Reverse2>
    requires(std::is_same_v<std::remove_const_t<BufferType1>,
                            std::remove_const_t<BufferType2>> &&
             ((Reverse1 && Reverse2) || (!Reverse1 && !Reverse2)))
constexpr auto
operator<=>(const CircularBufferIterator<BufferType1, Reverse1>& lhs,
            const CircularBufferIterator<BufferType2, Reverse2>& rhs) noexcept
{
    return lhs.m_index <=> rhs.m_index;
}

template <typename BufferType, bool Reverse>
constexpr auto
operator+(
    const typename CircularBufferIterator<BufferType, Reverse>::difference_type
        offset,
    const CircularBufferIterator<BufferType, Reverse>& it) noexcept
{
    return it + offset;
}

template <typename BufferType, bool Reverse>
constexpr auto
operator-(
    const typename CircularBufferIterator<BufferType, Reverse>::difference_type
        offset,
    const CircularBufferIterator<BufferType, Reverse>& it) noexcept
{
    auto temp = it;
    temp.m_index = (offset - it.m_index) %
        static_cast<typename CircularBufferIterator<BufferType,
                                                    Reverse>::difference_type>(
                       BufferType::max_size());
    return temp;
}

} // namespace circbuf
