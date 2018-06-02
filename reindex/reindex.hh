
#include <cassert>

namespace reindex {

template <
    typename _Impl, typename _BareContainerType, typename _KeyType,
    typename _UserKeyType = _KeyType>
class _resubscript_base {
  class _public_impl : public _Impl {
   public:
    using _Impl::_container;
    using _Impl::_reindex;
  };

 protected:
  inline constexpr const auto* _this() const noexcept {
    return static_cast<const _public_impl*>(this);
  }
  inline constexpr auto* _this() noexcept {
    return static_cast<_public_impl*>(this);
  }

 public:
  using type = _Impl;
  using user_key_type = _UserKeyType;
  using key_type = _KeyType;
  using bare_container_type = _BareContainerType;
  using value_type = typename bare_container_type::value_type;
  using size_type = typename bare_container_type::size_type;
  using reference = typename bare_container_type::reference;
  using const_reference = typename bare_container_type::const_reference;
  using iterator = typename bare_container_type::iterator;
  using const_iterator = typename bare_container_type::const_iterator;

  inline constexpr bare_container_type& container() noexcept {
    return _this()->_container;
  }
  inline constexpr const bare_container_type& container() const noexcept {
    return _this()->_container;
  }
  inline constexpr reference at(user_key_type index) {
    return container().at(_this()->_reindex(index));
  }
  inline constexpr const_reference at(user_key_type index) const {
    return container().at(_this()->_reindex(index));
  }
  inline constexpr reference operator[](user_key_type index) {
    return container().operator[](_this()->_reindex(index));
  }
  inline constexpr const_reference operator[](user_key_type index) const {
    return container().operator[](_this()->_reindex(index));
  }
};

template <
    typename _Impl, typename _ContainerType, typename _KeyType,
    typename _UserKeyType = _KeyType>
class resubscript_base
    : public _resubscript_base<_Impl, _ContainerType, _KeyType, _UserKeyType> {
 public:
  using container_type = _ContainerType;
  resubscript_base& operator=(resubscript_base&&) = default;
};

template <
    typename _Impl, typename _ContainerType, typename _KeyType,
    typename _UserKeyType>
class resubscript_base<_Impl, _ContainerType&, _KeyType, _UserKeyType>
    : public _resubscript_base<_Impl, _ContainerType, _KeyType, _UserKeyType> {
 public:
  using container_type = _ContainerType&;
  resubscript_base& operator=(const resubscript_base&) = default;
};

template <
    typename _Impl, typename _ContainerType, typename _KeyType = size_t,
    typename _UserKeyType = _KeyType>
class reindex_base
    : public resubscript_base<_Impl, _ContainerType, _KeyType, _UserKeyType> {
 protected:
  using _base = resubscript_base<
      reindex_base<_ContainerType, _KeyType, _UserKeyType>, _ContainerType,
      _KeyType, _UserKeyType>;
  static_assert(std::is_integral<typename resubscript_base<
                    reindex_base<_ContainerType, _KeyType, _UserKeyType>,
                    _ContainerType, _KeyType, _UserKeyType>::key_type>::value);

 public:
  using iterator = typename _base::iterator;
  using user_key_type = typename _base::user_key_type;
  using value_type = typename _base::value_type;

  template <class... Args>
  inline iterator emplace(user_key_type user_pos, Args&&... args) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().emplace(this->container().begin() + pos, args...);
  }
  inline iterator insert(user_key_type user_pos, const value_type& value) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().insert(this->container().begin() + pos, value);
  }
  inline iterator insert(user_key_type user_pos, value_type&& value) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().insert(
        this->container().begin() + pos, std::move(value));
  }
  template <class InputIt>
  inline iterator insert(user_key_type user_pos, InputIt first, InputIt last) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().insert(
        this->container().begin() + pos, first, last);
  }
  inline iterator insert(
      user_key_type user_pos, std::initializer_list<value_type> ilist) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().insert(this->container().begin() + pos, ilist);
  }
  inline iterator erase(user_key_type user_pos) {
    auto pos = this->_this()->_reindex(user_pos);
    return this->container().erase(this->container().begin() + pos);
  }
  inline iterator erase(user_key_type user_first, user_key_type user_last) {
    auto first = this->_this()->_reindex(user_first);
    auto last = this->_this()->_reindex(user_last);
    const auto begin = this->contaienr().begin();
    return this->container().erase(begin + first, begin + last);
  }
};

template <typename _base>
class bijective_mixin {
  class _public_impl : public _base::type {
   public:
    using _base::type::_container;
    using _base::type::_reindex;
    using _base::type::_reverse_reindex;
  };

 protected:
  inline constexpr const auto* _this() const noexcept {
    return static_cast<const _public_impl*>(this);
  }
  inline constexpr auto* _this() noexcept {
    return static_cast<_public_impl*>(this);
  }
};
template <typename _base>
class monotonic_mixin : public bijective_mixin<_base> {
 public:
  inline constexpr typename _base::user_key_type begin_index() const noexcept {
    return this->_this()->_reverse_reindex(0);
  }
  inline constexpr typename _base::user_key_type end_index() const noexcept {
    auto& container = this->_this()->container();
    return this->_this()->_reverse_reindex(container.size());
  }
};

template <
    typename _Impl, typename _ContainerType, typename _KeyType,
    typename _UserKeyType = _KeyType>
class remap_base
    : public resubscript_base<_Impl, _ContainerType, _KeyType, _UserKeyType> {
  using _base = resubscript_base<
      remap_base<_ContainerType, _KeyType, _UserKeyType>, _ContainerType,
      _KeyType, _UserKeyType>;
  static_assert(std::is_same<
                typename _base::container_type::key_type,
                typename _base::key_type>::value);

 public:
  using size_type = typename _base::size_type;
  using user_key_type = typename _base::user_key_type;
  inline size_type erase(const user_key_type& user_key) {
    return this->container().erase(this->_this()->_reindex(user_key));
  }
};

template <
    typename _T, typename _ContainerType = std::vector<_T>,
    typename _KeyType = size_t, typename _UserKeyType = _KeyType>
class rebase : public reindex_base<
                   rebase<_T, _ContainerType, _KeyType, _UserKeyType>,
                   _ContainerType, _KeyType, _UserKeyType>,
               public monotonic_mixin<reindex_base<
                   rebase<_T, _ContainerType, _KeyType, _UserKeyType>,
                   _ContainerType, _KeyType, _UserKeyType>> {
  using _base = reindex_base<
      rebase<_T, _ContainerType, _KeyType, _UserKeyType>, _ContainerType,
      _KeyType, _UserKeyType>;

 public:
  using size_type = typename _base::size_type;
  using key_type = typename _base::key_type;
  using user_key_type = typename _base::user_key_type;
  using container_type = typename _base::container_type;

 protected:
  // data fields
  key_type _offset;
  container_type _container;
  // end of data fields
 public:
  constexpr rebase(key_type offset, container_type& container) noexcept
      : _offset(offset), _container(container) {}
  template <typename... Args>
  constexpr rebase(key_type offset, Args... args)
      : _offset(offset), _container(args...) {}

 protected:
  inline constexpr const key_type _reindex(user_key_type user_index) const
      noexcept {
    key_type index = user_index - this->_offset;
#ifdef DEBUG
    assert(index < this->container().size());
#endif
    return index;
  }
  inline constexpr const user_key_type _reverse_reindex(key_type index) const
      noexcept {
    user_key_type user_index = this->_offset + index;
    return user_index;
  }
};

template <
    typename _T, typename _ContainerType = std::vector<_T>,
    typename _UserKeyType = size_t, typename _KeyType = _UserKeyType>
class slit : public reindex_base<
                 slit<_T, _ContainerType, _UserKeyType, _KeyType>,
                 _ContainerType, _UserKeyType, _KeyType>,
             public monotonic_mixin<reindex_base<
                 slit<_T, _ContainerType, _KeyType, _UserKeyType>,
                 _ContainerType, _KeyType, _UserKeyType>> {
  using _base = reindex_base<
      slit<_T, _ContainerType, _UserKeyType, _KeyType>, _ContainerType,
      _UserKeyType, _KeyType>;

 public:
  using container_type = typename _base::container_type;
  using size_type = typename _base::size_type;
  using key_type = typename _base::key_type;
  using user_key_type = typename _base::user_key_type;

 protected:
  // data fields
  key_type _offset;
  size_type _step;
  container_type _container;
  // end of data fields
 public:
  constexpr slit(
      key_type offset, size_type step, container_type& container) noexcept
      : _offset(offset), _step(step), _container(container) {}
  template <typename... Args>
  constexpr slit(key_type offset, size_type step, Args... arg)
      : _offset(offset), _step(step), _container(arg...) {}

 protected:
  inline constexpr const size_type _reindex(user_key_type user_index) const
      noexcept {
    key_type index = (user_index - this->_offset) / this->_step;
#ifdef DEBUG
    assert(user_index % this->_step == 0);
    assert(index < this->_container.size());
#endif
    return index;
  }
  inline constexpr const user_key_type _reverse_reindex(key_type index) const
      noexcept {
    user_key_type user_index = this->_offset + (index * this->_step);
    return user_index;
  }
};
static_assert(std::is_same<slit<int>::type, slit<int>>::value);

}  // namespace reindex
