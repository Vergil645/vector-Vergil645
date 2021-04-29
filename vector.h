#pragma once
#include <cstddeftemplate <typename T>
struct vector {
  using iterator = T*;
  using const_iterator = T const*;

  vector();                               // O(1) nothrow
  vector(vector const&);                  // O(N) strong
  vector& operator=(vector const& other); // O(N) strong

  ~vector(); // O(N) nothrow

  T& operator[](size_t i);             // O(1) nothrow
  T const& operator[](size_t i) const; // O(1) nothrow

  T* data();             // O(1) nothrow
  T const* data() const; // O(1) nothrow
  size_t size() const;   // O(1) nothrow

  T& front();             // O(1) nothrow
  T const& front() const; // O(1) nothrow

  T& back();                // O(1) nothrow
  T const& back() const;    // O(1) nothrow
  void push_back(T const&); // O(1)* strong
  void pop_back();          // O(1) nothrow

  bool empty() const; // O(1) nothrow

  size_t capacity() const; // O(1) nothrow
  void reserve(size_t);    // O(N) strong
  void shrink_to_fit();    // O(N) strong

  void clear(); // O(N) nothrow

  void swap(vector&); // O(1) nothrow

  iterator begin(); // O(1) nothrow
  iterator end();   // O(1) nothrow

  const_iterator begin() const; // O(1) nothrow
  const_iterator end() const;   // O(1) nothrow

  iterator insert(const_iterator pos, T const&); // O(N) strong
  
  iterator erase(const_iterator pos); // O(N) nothrow(swap)

  iterator erase(const_iterator first, const_iterator last); // O(N) nothrow(swap)

private:
  T* data;
  size_t size;
  size_t capacity;
}; 
