#pragma once
#include <cstddef>

template <typename T>
struct vector {
  using iterator = T*;
  using const_iterator = T const*;

  vector() noexcept = default;

  vector(vector const& other) : vector() {
    change_capacity(other.size_);
    for (size_t i = 0; i != other.size_; i++) {
      push_back(other[i]);
    }
  }

  vector& operator=(vector const& other) {
    vector(other).swap(*this);
    return *this;
  }

  ~vector() noexcept {
    clear_array_prefix(data_, size_);
    operator delete(data_);
  }

  T& operator[](size_t i) noexcept {
    return data_[i];
  }

  T const& operator[](size_t i) const noexcept {
    return data_[i];
  }

  T* data() noexcept {
    return data_;
  }

  T const* data() const noexcept {
    return data_;
  }

  size_t size() const noexcept {
    return size_;
  }

  T& front() noexcept {
    return data_[0];
  }

  T const& front() const noexcept {
    return data_[0];
  }

  T& back() noexcept {
    return data_[size_ - 1];
  }

  T const& back() const noexcept {
    return data_[size_ - 1];
  }

  void push_back(T const& value) {
    if (size_ == capacity_) {
      size_t val_pos = &value - begin();
      change_capacity(capacity_ == 0 ? 1 : capacity_ * 2);
      new (end()) T(val_pos >= 0 && val_pos < size_ ? data_[val_pos] : value);
    } else {
      new (end()) T(value);
    }
    ++size_;
  }

  void pop_back() noexcept {
    --size_;
    data_[size_].~T();
  }

  bool empty() const noexcept {
    return size_ == 0;
  }

  size_t capacity() const noexcept {
    return capacity_;
  }

  void reserve(size_t new_cap) {
    if (capacity_ < new_cap) {
      change_capacity(new_cap);
    }
  }

  void shrink_to_fit() {
    if (size_ != capacity_) {
      change_capacity(size_);
    }
  }

  void clear() noexcept {
    clear_array_prefix(data_, size_);
    size_ = 0;
  }

  void swap(vector& other) noexcept {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
  }

  iterator begin() noexcept {
    return data_;
  }

  iterator end() noexcept {
    return data_ + size_;
  }

  const_iterator begin() const noexcept {
    return data_;
  }

  const_iterator end() const noexcept {
    return data_ + size_;
  }

  iterator insert(const_iterator pos, T const& value) {
    size_t pos_index = pos - begin();
    push_back(value);
    for (size_t i = size_ - 1; i != pos_index; i--) {
      std::swap(data_[i - 1], data_[i]);
    }
    return begin() + pos_index;
  }

  iterator erase(const_iterator pos) noexcept {
    return pos == end() ? end() : erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return const_cast<iterator>(first);
    } else {
      size_t first_index = first - begin();
      size_t count = last - first;
      for (size_t i = first_index; i != size_ - count; i++) {
        std::swap(data_[i], data_[i + count]);
      }
      for (size_t i = 0; i != count; i++) {
        pop_back();
      }
      return begin() + first_index;
    }
  }

private:
  T* data_{nullptr};
  size_t size_{0};
  size_t capacity_{0};

  static void clear_array_prefix(T* data_array, size_t prefix_size) {
    for (size_t i = 0; i != prefix_size; i++) {
      data_array[i].~T();
    }
  }

  void change_capacity(size_t new_cap) {
    if (new_cap == 0) {
      operator delete(data_);
      data_ = nullptr;
    } else {
      T* new_data = static_cast<T*>(operator new(new_cap * sizeof(T)));
      for (size_t i = 0; i != size_; i++) {
        try {
          new (new_data + i) T(data_[i]);
        } catch (...) {
          clear_array_prefix(new_data, i);
          operator delete(new_data);
          throw;
        }
      }
      clear_array_prefix(data_, size_);
      operator delete(data_);
      data_ = new_data;
    }
    capacity_ = new_cap;
  }
};
