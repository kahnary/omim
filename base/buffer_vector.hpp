#pragma once

#include "../std/algorithm.hpp"
#include "../std/vector.hpp"
#include "../base/assert.hpp"
#include "../base/swap.hpp"

template <class T, size_t N> class buffer_vector
{
private:
  enum { USE_DYNAMIC = N + 1 };
  T m_static[N];
  size_t m_size;
  vector<T> m_dynamic;

public:
  typedef T value_type;
  typedef T const & const_reference;
  typedef T & reference;
  typedef size_t size_type;
  typedef T const * const_iterator;
  typedef T * iterator;

  buffer_vector() : m_size(0) {}
  explicit buffer_vector(size_t n, T c = T()) : m_size(0)
  {
    resize(n, c);
  }

  template <typename IterT>
  explicit buffer_vector(IterT beg, IterT end) : m_size(0)
  {
    assign(beg, end);
  }

  template <typename IterT>
  void append(IterT beg, IterT end)
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.insert(m_dynamic.end(), beg, end);
    else
    {
      while (beg != end)
      {
        if (m_size == N)
        {
          m_dynamic.reserve(N * 2);
          SwitchToDynamic();
          while (beg != end)
            m_dynamic.push_back(*beg++);
          break;
        }
        m_static[m_size++] = *beg++;
      }
    }
  }

  template <typename IterT>
  void assign(IterT beg, IterT end)
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.assign(beg, end);
    else
    {
      m_size = 0;
      append(beg, end);
    }
  }

  void reserve(size_t n)
  {
    if (m_size == USE_DYNAMIC || n > N)
      m_dynamic.reserve(n);
  }

  void resize_no_init(size_t n)
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.resize(n);
    else
    {
      if (n <= N)
        m_size = n;
      else
      {
        m_dynamic.reserve(n);
        SwitchToDynamic();
        m_dynamic.resize(n);
        ASSERT_EQUAL(m_dynamic.size(), n, ());
      }
    }
  }

  void resize(size_t n, T c = T())
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.resize(n, c);
    else
    {
      if (n <= N)
      {
        for (size_t i = m_size; i < n; ++i)
          m_static[i] = c;
        m_size = n;
      }
      else
      {
        m_dynamic.reserve(n);
        size_t const oldSize = m_size;
        SwitchToDynamic();
        m_dynamic.insert(m_dynamic.end(), n - oldSize, c);
        ASSERT_EQUAL(m_dynamic.size(), n, ());
      }
    }
  }

  void clear()
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.clear();
    else
      m_size = 0;
  }

  /// @todo Here is some inconsistencies:
  /// - "data" method should return 0 if vector is empty;\n
  /// - potential memory overrun if m_dynamic is empty;\n
  /// The best way to fix this is to reset m_size from USE_DYNAMIC to 0 when vector becomes empty.
  /// But now I will just add some assertions to test memory overrun.
  //@{
  T const * data() const
  {
    if (m_size == USE_DYNAMIC)
    {
      ASSERT ( !m_dynamic.empty(), () );
      return &m_dynamic[0];
    }
    else
      return &m_static[0];
  }

  T * data()
  {
    if (m_size == USE_DYNAMIC)
    {
      ASSERT ( !m_dynamic.empty(), () );
      return &m_dynamic[0];
    }
    else
      return &m_static[0];
  }
  //@}

  T const * begin() const { return data(); }
  T       * begin()       { return data(); }
  T const * end() const { return data() + size(); }
  T       * end()       { return data() + size(); }
  //@}

  bool empty() const { return (m_size == USE_DYNAMIC ? m_dynamic.empty() : m_size == 0); }
  size_t size() const { return (m_size == USE_DYNAMIC ? m_dynamic.size() : m_size); }

  T const & front() const
  {
    ASSERT(!empty(), ());
    return *begin();
  }
  T & front()
  {
    ASSERT(!empty(), ());
    return *begin();
  }
  T const & back() const
  {
    ASSERT(!empty(), ());
    return *(end() - 1);
  }
  T & back()
  {
    ASSERT(!empty(), ());
    return *(end() - 1);
  }

  T const & operator[](size_t i) const
  {
    ASSERT_LESS(i, size(), ());
    return *(begin() + i);
  }
  T & operator[](size_t i)
  {
    ASSERT_LESS(i, size(), ());
    return *(begin() + i);
  }

  void swap(buffer_vector<T, N> & rhs)
  {
    m_dynamic.swap(rhs.m_dynamic);
    Swap(m_size, rhs.m_size);
    for (size_t i = 0; i < N; ++i)
      Swap(m_static[i], rhs.m_static[i]);
  }

  void push_back(T const & t)
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.push_back(t);
    else
    {
      if (m_size < N)
        m_static[m_size++] = t;
      else
      {
        ASSERT_EQUAL(m_size, N, ());
        m_dynamic.reserve(N + 1);
        SwitchToDynamic();
        m_dynamic.push_back(t);
        ASSERT_EQUAL(m_dynamic.size(), N + 1, ());
      }
    }
  }

  void pop_back()
  {
    if (m_size == USE_DYNAMIC)
      m_dynamic.pop_back();
    else
    {
      ASSERT_GREATER(m_size, 0, ());
      --m_size;
    }
  }

  template <typename IterT> void insert(const_iterator where, IterT beg, IterT end)
  {
    ptrdiff_t const pos = where - data();
    ASSERT_GREATER_OR_EQUAL(pos, 0, ());
    ASSERT_LESS_OR_EQUAL(pos, static_cast<ptrdiff_t>(size()), ());

    if (m_size == USE_DYNAMIC)
      m_dynamic.insert(m_dynamic.begin() + pos, beg, end);
    else
    {
      size_t const n = end - beg;
      if (m_size + n <= N)
      {
        if (pos != m_size)
          for (ptrdiff_t i = m_size - 1; i >= pos; --i)
            Swap(m_static[i], m_static[i + n]);

        m_size += n;
        T * writableWhere = &m_static[0] + pos;
        ASSERT_EQUAL(where, writableWhere, ());
        while (beg != end)
          *(writableWhere++) = *(beg++);
      }
      else
      {
        m_dynamic.reserve(m_size + n);
        SwitchToDynamic();
        m_dynamic.insert(m_dynamic.begin() + pos, beg, end);
      }
    }
  }

private:
  void SwitchToDynamic()
  {
    ASSERT_NOT_EQUAL(m_size, static_cast<size_t>(USE_DYNAMIC), ());
    ASSERT_EQUAL(m_dynamic.size(), 0, ());
    m_dynamic.insert(m_dynamic.end(), m_size, T());
    for (size_t i = 0; i < m_size; ++i)
      Swap(m_static[i], m_dynamic[i]);
    m_size = USE_DYNAMIC;
  }
};

template <class T, size_t N>
void swap(buffer_vector<T, N> & r1, buffer_vector<T, N> & r2)
{
  r1.swap(r2);
}

template <typename T, size_t N>
inline string DebugPrint(buffer_vector<T, N> const & v)
{
  return ::my::impl::DebugPrintSequence(v.data(), v.data() + v.size());
}

template <typename T, size_t N1, size_t N2>
inline bool operator==(buffer_vector<T, N1> const & v1, buffer_vector<T, N2> const & v2)
{
  return (v1.size() == v2.size() && std::equal(v1.begin(), v1.end(), v2.begin()));
}

template <typename T, size_t N1, size_t N2>
inline bool operator!=(buffer_vector<T, N1> const & v1, buffer_vector<T, N2> const & v2)
{
  return !(v1 == v2);
}

template <typename T, size_t N1, size_t N2>
inline bool operator<(buffer_vector<T, N1> const & v1, buffer_vector<T, N2> const & v2)
{
  return lexicographical_compare(v1.begin(), v1.end(), v2.begin(), v2.end());
}
