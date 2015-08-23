#ifndef ITERATOR_ITERATOR_HPP_
#define ITERATOR_ITERATOR_HPP_

#include "iterbase.hpp"
#include "zip.hpp"
#include <iterator>
#include <type_traits>
#include <utility>

// IterIterWrapper and IteratorIterator provide a means to have a container
// of iterators act like a container of the pointed to objects. This is useful
// for combinatorics and similar itertools which need to keep track of
// more than one element at a time.
// an IterIterWrapper<some_collection_type<collection<T>::iterator>>
// behave like some_collection<T> when iterated over or indexed

namespace iter {
  namespace impl {
    template <typename T, typename = void>
    struct HasConstDeref : std::false_type {};

    template <typename T>
    struct HasConstDeref<T, void_t<decltype(*std::declval<const T&>())>>
        : std::true_type {};

    template <typename Iter,
        typename Diff = typename std::iterator_traits<Iter>::difference_type>
    class IteratorIterator
        : public std::iterator<std::random_access_iterator_tag,
              typename std::iterator_traits<Iter>::value_type, Diff,
              typename std::iterator_traits<Iter>::pointer,
              typename std::iterator_traits<Iter>::reference> {
      static_assert(
          std::is_same<typename std::iterator_traits<Iter>::iterator_category,
              std::random_access_iterator_tag>::value,
          "IteratorIterator only works with random access iterators");

     private:
      Iter sub_iter;

     public:
      IteratorIterator() = default;
      IteratorIterator(const Iter& it) : sub_iter{it} {}

      bool operator==(const IteratorIterator& other) const {
        return !(*this != other);
      }

      bool operator!=(const IteratorIterator& other) const {
        return this->sub_iter != other.sub_iter;
      }

      IteratorIterator& operator++() {
        ++this->sub_iter;
        return *this;
      }

      IteratorIterator operator++(int) {
        auto ret = *this;
        ++*this;
        return ret;
      }

      IteratorIterator& operator--() {
        --this->sub_iter;
        return *this;
      }

      IteratorIterator operator--(int) {
        auto ret = *this;
        --*this;
        return ret;
      }

      auto operator*() -> decltype(**sub_iter) {
        return **this->sub_iter;
      }

      auto operator -> () -> decltype(*sub_iter) {
        return *this->sub_iter;
      }

      IteratorIterator& operator+=(Diff n) {
        this->sub_iter += n;
        return *this;
      }

      IteratorIterator operator+(Diff n) const {
        auto it = *this;
        it += n;
        return it;
      }

      friend IteratorIterator operator+(Diff n, IteratorIterator it) {
        it += n;
        return it;
      }

      IteratorIterator& operator-=(Diff n) {
        this->sub_iter -= n;
        return *this;
      }

      IteratorIterator operator-(Diff n) const {
        auto it = *this;
        it -= n;
        return it;
      }

      friend IteratorIterator operator-(Diff n, IteratorIterator it) {
        it -= n;
        return it;
      }

      Diff operator-(const IteratorIterator& rhs) const {
        return this->sub_iter - rhs.sub_iter;
      }

      auto operator[](Diff idx) -> decltype(*sub_iter[idx]) {
        return *sub_iter[idx];
      }

      bool operator<(const IteratorIterator& rhs) const {
        return this->sub_iter < rhs.sub_iter;
      }

      bool operator>(const IteratorIterator& rhs) const {
        return this->sub_iter > rhs.sub_iter;
      }

      bool operator<=(const IteratorIterator& rhs) const {
        return this->sub_iter <= rhs.sub_iter;
      }

      bool operator>=(const IteratorIterator& rhs) const {
        return this->sub_iter >= rhs.sub_iter;
      }
    };

    template <typename Container>
    class IterIterWrapper {
     private:
      Container container;

      using contained_iter = typename Container::value_type;
      using size_type = typename Container::size_type;
      using iterator = IteratorIterator<typename Container::iterator>;
      using const_iterator =
          IteratorIterator<typename Container::const_iterator>;
      using reverse_iterator =
          IteratorIterator<typename Container::reverse_iterator>;
      using const_reverse_iterator =
          IteratorIterator<typename Container::const_reverse_iterator>;

      template <typename U = Container, typename = void>
      struct ConstAtTypeOrVoid : type_is<void> {};

      template <typename U>
      struct ConstAtTypeOrVoid<U,
          void_t<decltype(*std::declval<const U&>().at(0))>>
          : type_is<decltype(*std::declval<const U&>().at(0))> {};

      using const_at_type_or_void_t = typename ConstAtTypeOrVoid<>::type;

      template <typename U = Container, typename = void>
      struct ConstIndexTypeOrVoid : type_is<void> {};

      template <typename U>
      struct ConstIndexTypeOrVoid<U,
          void_t<decltype(*std::declval<const U&>()[0])>>
          : type_is<decltype(*std::declval<const U&>()[0])> {};

      using const_index_type_or_void_t = typename ConstIndexTypeOrVoid<>::type;

     public:
      IterIterWrapper() = default;

      explicit IterIterWrapper(size_type sz) : container(sz) {}

      IterIterWrapper(size_type sz, const contained_iter& val)
          : container(sz, val) {}

      auto at(size_type pos) -> decltype(*container.at(pos)) {
        return *container.at(pos);
      }

      auto at(size_type pos) const -> const_at_type_or_void_t {
        return *container.at(pos);
      }

      auto operator[](size_type pos) noexcept(noexcept(*container[pos]))
          -> decltype(*container[pos]) {
        return *container[pos];
      }

      auto operator[](size_type pos) const noexcept(noexcept(*container[pos]))
          -> const_index_type_or_void_t {
        return *container[pos];
      }

      bool empty() const noexcept {
        return container.empty();
      }

      size_type size() const noexcept {
        return container.size();
      }

      iterator begin() noexcept {
        return {container.begin()};
      }

      iterator end() noexcept {
        return {container.end()};
      }

      const_iterator begin() const noexcept {
        return {container.begin()};
      }

      const_iterator end() const noexcept {
        return {container.end()};
      }

      const_iterator cbegin() const noexcept {
        return {container.cbegin()};
      }

      const_iterator cend() const noexcept {
        return {container.cend()};
      }

      reverse_iterator rbegin() noexcept {
        return {container.rbegin()};
      }

      reverse_iterator rend() noexcept {
        return {container.rend()};
      }

      const_reverse_iterator rbegin() const noexcept {
        return {container.rbegin()};
      }

      const_reverse_iterator rend() const noexcept {
        return {container.rend()};
      }

      const_reverse_iterator crbegin() const noexcept {
        return {container.rbegin()};
      }

      const_reverse_iterator crend() const noexcept {
        return {container.rend()};
      }

      // get() exposes the underlying container.  this allows the
      // itertools to manipulate the iterators in the container
      // and should not be depended on anywhere else.
      Container& get() noexcept {
        return container;
      }

      const Container& get() const noexcept {
        return container;
      }
    };
  }
}

#endif