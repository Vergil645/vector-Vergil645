#include <unordered_set>

#include "gtest/gtest.h"

#include "vector.h"

template struct vector<int>;

template <typename T>
T const& as_const(T& obj) {
  return obj;
}

template <typename T>
struct element {
  element() {
    add_instance();
  }

  element(T const& val) : val(val) {
    add_instance();
  }

  element(element const& rhs) : val(rhs.val) {
    copy();
    add_instance();
  }

  element& operator=(element const& rhs) {
    assert_exists();
    rhs.assert_exists();
    copy();
    val = rhs.val;
    return *this;
  }

  ~element() {
    delete_instance();
  }

  static std::unordered_set<element const*>& instances() {
    static std::unordered_set<element const*> instances;
    return instances;
  }

  static void expect_no_instances() {
    if (!instances().empty()) {
      FAIL() << "not all instances are destroyed";
      instances().clear();
    }
  }

  static void set_throw_countdown(size_t val) {
    throw_countdown = val;
  }

  friend bool operator==(element const& a, element const& b) {
    return a.val == b.val;
  }

  friend bool operator!=(element const& a, element const& b) {
    return a.val != b.val;
  }

private:
  void add_instance() {
    auto p = instances().insert(this);
    if (!p.second) {
      FAIL() << "a new object is created at the address "
             << static_cast<void*>(this)
             << " while the previous object at this address was not destroyed";
    }
  }

  void delete_instance() {
    size_t erased = instances().erase(this);
    if (erased != 1) {
      FAIL() << "attempt of destroying non-existing object at address "
             << static_cast<void*>(this);
    }
  }

  void assert_exists() const {
    std::unordered_set<element const*> const& inst = instances();
    bool exists = inst.find(this) != inst.end();
    if (!exists) {
      FAIL() << "accessing an non-existsing object at address "
             << static_cast<void const*>(this);
    }
  }

  void copy() {
    if (throw_countdown != 0) {
      --throw_countdown;
      if (throw_countdown == 0)
        throw std::runtime_error("copy failed");
    }
  }

private:
  T val;
  static size_t throw_countdown;
};

template <typename T>
size_t element<T>::throw_countdown = 0;

TEST(correctness, default_ctor) {
  vector<element<int>> a;
  element<int>::expect_no_instances();
  EXPECT_TRUE(a.empty());
  EXPECT_EQ(0, a.size());
}

TEST(correctness, push_back) {
  size_t const N = 5000;
  {
    vector<element<size_t>> a;
    for (size_t i = 0; i != N; ++i)
      a.push_back(i);

    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(i, a[i]);
  }

  element<size_t>::expect_no_instances();
}

TEST(correctness, push_back_from_self) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    a.push_back(42);
    for (size_t i = 0; i != N; ++i)
      a.push_back(a[0]);

    for (size_t i = 0; i != a.size(); ++i)
      EXPECT_EQ(42, a[i]);
  }

  element<size_t>::expect_no_instances();
}

TEST(correctness, subscription) {
  size_t const N = 500;
  vector<size_t> a;
  for (size_t i = 0; i != N; ++i)
    a.push_back(2 * i + 1);

  for (size_t i = 0; i != N; ++i)
    EXPECT_EQ(2 * i + 1, a[i]);

  vector<size_t> const& ca = a;

  for (size_t i = 0; i != N; ++i)
    EXPECT_EQ(2 * i + 1, ca[i]);
}

TEST(correctness, data) {
  size_t const N = 500;
  vector<element<size_t>> a;

  for (size_t i = 0; i != N; ++i)
    a.push_back(2 * i + 1);

  {
    element<size_t>* ptr = a.data();
    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * i + 1, ptr[i]);
  }

  {
    element<size_t> const* cptr = as_const(a).data();
    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * i + 1, cptr[i]);
  }
}

TEST(correctness, front_back) {
  size_t const N = 500;
  vector<element<size_t>> a;
  for (size_t i = 0; i != N; ++i)
    a.push_back(2 * i + 1);

  EXPECT_EQ(1, a.front());
  EXPECT_EQ(1, as_const(a).front());

  EXPECT_EQ(999, a.back());
  EXPECT_EQ(999, as_const(a).back());
}

TEST(correctness, capacity) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    a.reserve(N);
    EXPECT_LE(N, a.capacity());
    for (size_t i = 0; i != N - 1; ++i)
      a.push_back(2 * i + 1);
    EXPECT_LE(N, a.capacity());
    a.shrink_to_fit();
    EXPECT_EQ(N - 1, a.capacity());
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, superfluous_reserve) {
  size_t const N = 500, K = 100;
  {
    vector<element<size_t>> a;
    a.reserve(N);
    EXPECT_GE(a.capacity(), N);
    a.reserve(K);
    EXPECT_GE(a.capacity(), N);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, clear) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);
    size_t c = a.capacity();
    a.clear();
    EXPECT_EQ(c, a.capacity());
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, superfluous_shrink_to_fit) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    a.reserve(N);
    size_t c = a.capacity();
    for (size_t i = 0; i != c; ++i)
      a.push_back(2 * i + 1);
    element<size_t>* old_data = a.data();
    a.shrink_to_fit();
    EXPECT_EQ(old_data, a.data());
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, copy_ctor) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    for (size_t i = 0; i != N; ++i)
      a.push_back(i);

    vector<element<size_t>> b = a;
    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(i, b[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, assignment_operator) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);

    vector<element<size_t>> b;
    b.push_back(42);

    b = a;
    EXPECT_EQ(N, b.size());
    for (size_t i = 0; i != N; ++i) {
      auto tmp = b[i];
      EXPECT_EQ(2 * i + 1, tmp);
    }
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, self_assignment) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;
    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);
    a = a;

    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * i + 1, a[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, pop_back) {
  size_t const N = 500;
  vector<element<size_t>> a;

  for (size_t i = 0; i != N; ++i)
    a.push_back(2 * i + 1);

  for (size_t i = N; i != 0; --i) {
    EXPECT_EQ(2 * i - 1, a.back());
    EXPECT_EQ(i, a.size());
    a.pop_back();
  }
  EXPECT_TRUE(a.empty());
  element<size_t>::expect_no_instances();
}

TEST(correctness, insert_begin) {
  size_t const N = 500;
  vector<element<size_t>> a;

  for (size_t i = 0; i != N; ++i)
    a.insert(a.begin(), i);

  for (size_t i = 0; i != N; ++i) {
    EXPECT_EQ(i, a.back());
    a.pop_back();
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, insert_end) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);
    EXPECT_EQ(N, a.size());

    for (size_t i = 0; i != N; ++i) {
      EXPECT_EQ(N + i, a.size());
      a.insert(a.end(), 4 * i + 1);
      EXPECT_EQ(4 * i + 1, a.back());
    }

    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * i + 1, a[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(performance, insert) {
  const size_t N = 10000;
  vector<vector<int>> a;

  for (size_t i = 0; i < N; ++i) {
    a.push_back(vector<int>());
    for (size_t j = 0; j < N; ++j) {
      a.back().push_back(j);
    }
  }

  vector<int> temp;
  for (size_t i = 0; i < N; ++i) {
    temp.push_back(i);
  }
  a.insert(a.begin(), temp);
}

TEST(correctness, erase) {
  size_t const N = 500;
  {
    for (size_t i = 0; i != N; ++i) {
      vector<element<size_t>> a;
      for (size_t j = 0; j != N; ++j)
        a.push_back(2 * j + 1);

      a.erase(a.begin() + i);
      size_t cnt = 0;
      for (size_t j = 0; j != N - 1; ++j) {
        if (j == i)
          ++cnt;
        EXPECT_EQ(2 * cnt + 1, a[j]);
        ++cnt;
      }
    }
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_begin) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != 2 * N; ++i)
      a.push_back(2 * i + 1);

    for (size_t i = 0; i != N; ++i)
      a.erase(a.begin());

    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * (i + N) + 1, a[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_end) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != 2 * N; ++i)
      a.push_back(2 * i + 1);

    for (size_t i = 0; i != N; ++i)
      a.erase(a.end() - 1);

    for (size_t i = 0; i != N; ++i)
      EXPECT_EQ(2 * i + 1, a[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_range_begin) {
  size_t const N = 500, K = 100;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);

    a.erase(a.begin(), a.begin() + K);

    for (size_t i = 0; i != N - K; ++i)
      EXPECT_EQ(2 * (i + K) + 1, a[i]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_range_middle) {
  size_t const N = 500, K = 100;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);

    a.erase(a.begin() + K, a.end() - K);

    for (size_t i = 0; i != K; ++i)
      EXPECT_EQ(2 * i + 1, a[i]);
    for (size_t i = 0; i != K; ++i)
      EXPECT_EQ(2 * (i + N - K) + 1, a[i + K]);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_range_end) {
  size_t const N = 500, K = 100;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);

    a.erase(a.end() - K, a.end());
    for (size_t i = 0; i != N - K; ++i)
      EXPECT_EQ(2 * i + 1, a[i]);
  }

  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_range_all) {
  size_t const N = 500;
  {
    vector<element<size_t>> a;

    for (size_t i = 0; i != N; ++i)
      a.push_back(2 * i + 1);

    a.erase(a.begin(), a.end());

    EXPECT_TRUE(a.empty());
  }

  element<size_t>::expect_no_instances();
}

TEST(correctness, erase_big_range) {
  {
    vector<element<size_t>> c;
    for (size_t i = 0; i != 100; ++i) {
      for (size_t j = 0; j != 50000; ++j)
        c.push_back(j);
      c.erase(c.begin() + 100, c.end() - 100);
      c.clear();
    }
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, reallocation_throw) {
  {
    vector<element<size_t>> a;
    a.reserve(10);
    size_t n = a.capacity();
    for (size_t i = 0; i != n; ++i)
      a.push_back(i);
    element<size_t>::set_throw_countdown(7);
    EXPECT_THROW(a.push_back(42), std::runtime_error);
  }
  element<size_t>::expect_no_instances();
}

TEST(correctness, empty_storage) {
  vector<int> a;
  EXPECT_EQ(nullptr, a.data());
  vector<int> b = a;
  EXPECT_EQ(nullptr, b.data());
  a = b;
  EXPECT_EQ(nullptr, a.data());
}

TEST(correctness, empty_storage_shrink_to_fit) {
  vector<int> a;
  a.push_back(42);
  a.pop_back();
  EXPECT_NE(nullptr, a.data());
  a.shrink_to_fit();
  EXPECT_EQ(nullptr, a.data());
}

// This test actually checks memory leak in pair with @valgrind
TEST(correctness, copy_throw) {
  vector<element<size_t>> a;
  a.reserve(10);
  size_t n = a.capacity();
  for (size_t i = 0; i != n; ++i)
    a.push_back(i);
  element<size_t>::set_throw_countdown(7);
  EXPECT_THROW({ vector<element<size_t>> b(a); }, std::runtime_error);
}

TEST(correctness, iter_types) {
  using el_t = element<size_t>;
  using vec_t = vector<el_t>;
  bool test1 = std::is_same<el_t*, typename vec_t::iterator>::value;
  bool test2 = std::is_same<el_t const*, typename vec_t::const_iterator>::value;
  EXPECT_TRUE(test1);
  EXPECT_TRUE(test2);
}

// Expect no extra allocation
TEST(correctness, ctor_alloc) {
  vector<element<size_t>> a;
  a.reserve(10);
  a.push_back(5);

  auto b = a;
  EXPECT_EQ(1, b.capacity());
}
