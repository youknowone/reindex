#ifndef __REINDEX_HH__
#define __REINDEX_HH__

#include <cassert>

namespace reindex {

template <typename _Struct, typename _Impl>
class converter_base : public _Struct {
    class _public_impl : public _Impl {
       public:
        using _Impl::_convert;
    };
    using _struct = _Struct;

   protected:
    inline constexpr auto* _this() { return static_cast<_public_impl*>(this); }
    inline constexpr const auto* _this() const {
        return static_cast<const _public_impl*>(this);
    }

   public:
    using type = _Impl;
    inline constexpr const typename _struct::key_type convert(
            typename _struct::user_key_type user_index) const noexcept {
        return _this()->_convert(user_index);
    }
};

template <typename _Struct, typename _Impl>
class bijective_converter_mixin {
    class _public_impl : public _Impl {
       public:
        using _Impl::_convert;
        using _Impl::_revert;
    };

   protected:
    inline constexpr auto* _this() { return static_cast<_public_impl*>(this); }
    inline constexpr const auto* _this() const {
        return static_cast<const _public_impl*>(this);
    }

   public:
    inline constexpr const typename _Struct::user_key_type revert(
            typename _Struct::key_type index) const noexcept {
        return _this()->_revert(index);
    }
};

template <typename _Struct, typename _Impl>
class monotonic_converter_mixin
        : public bijective_converter_mixin<_Struct, _Impl> {
   public:
    inline constexpr typename _Struct::key_type monotonic_convert(
            typename _Struct::user_key_type user_key) const noexcept {
        return this->convert(user_key);
    }
    inline constexpr typename _Struct::user_key_type monotonic_revert(
            typename _Struct::key_type key) const noexcept {
        return this->revert(key);
    }
};

template <typename _KeyType, typename _UserKeyType>
struct _rebase_converter {
    using key_type = _KeyType;
    using user_key_type = _UserKeyType;

    user_key_type offset;
};

template <typename _KeyType = size_t, typename _UserKeyType = _KeyType>
class rebase_converter : public converter_base<
                                 _rebase_converter<_KeyType, _UserKeyType>,
                                 rebase_converter<_KeyType, _UserKeyType>>,
                         public monotonic_converter_mixin<
                                 _rebase_converter<_KeyType, _UserKeyType>,
                                 rebase_converter<_KeyType, _UserKeyType>> {
    using _base = converter_base<
            _rebase_converter<_KeyType, _UserKeyType>,
            rebase_converter<_KeyType, _UserKeyType>>;

   public:
    using typename _base::key_type;
    using typename _base::user_key_type;

   protected:
    inline constexpr const key_type _convert(user_key_type user_index) const
            noexcept {
        key_type index = user_index - this->offset;
        return index;
    }
    inline constexpr const user_key_type _revert(key_type index) const
            noexcept {
        user_key_type user_index = this->offset + index;
        return user_index;
    }
};

template <typename _KeyType, typename _UserKeyType>
struct _slit_converter {
    using key_type = _KeyType;
    using user_key_type = _UserKeyType;

    user_key_type offset;
    user_key_type step;
};

template <typename _KeyType = size_t, typename _UserKeyType = _KeyType>
class slit_converter : public converter_base<
                               _slit_converter<_KeyType, _UserKeyType>,
                               slit_converter<_KeyType, _UserKeyType>>,
                       public monotonic_converter_mixin<
                               _slit_converter<_KeyType, _UserKeyType>,
                               slit_converter<_KeyType, _UserKeyType>> {
    using _base = converter_base<
            _slit_converter<_KeyType, _UserKeyType>,
            slit_converter<_KeyType, _UserKeyType>>;

   public:
    using typename _base::key_type;
    using typename _base::user_key_type;

   protected:
    inline constexpr const key_type _convert(user_key_type user_index) const
            noexcept {
        key_type index = (user_index - this->offset) / this->step;
#ifdef DEBUG
        assert((user_index - this->offset) % this->step == 0);
#endif
        return index;
    }
    inline constexpr const user_key_type _revert(key_type index) const
            noexcept {
        user_key_type user_index = this->offset + (index * this->step);
        return user_index;
    }
};

template <typename _Impl, typename _BareContainerType, typename _ConverterType>
class _resubscript_base {
    class _public_impl : public _Impl {
       public:
        using _Impl::_container;
        using _Impl::_converter;
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
    using converter_type = _ConverterType;
    using user_key_type = typename converter_type::user_key_type;
    using key_type = typename converter_type::key_type;
    using bare_container_type = _BareContainerType;
    using value_type = typename bare_container_type::value_type;
    using size_type = typename bare_container_type::size_type;
    using reference = typename bare_container_type::reference;
    using const_reference = typename bare_container_type::const_reference;
    using iterator = typename bare_container_type::iterator;
    using const_iterator = typename bare_container_type::const_iterator;

    inline constexpr converter_type& converter() noexcept {
        return _this()->_converter;
    }
    inline constexpr const converter_type& converter() const noexcept {
        return _this()->_converter;
    }
    inline constexpr bare_container_type& container() noexcept {
        return _this()->_container;
    }
    inline constexpr const bare_container_type& container() const noexcept {
        return _this()->_container;
    }
    inline constexpr key_type convert_key(user_key_type index) const noexcept {
        return _this()->_converter.convert(index);
    }
    inline constexpr reference at(user_key_type index) {
        return container().at(convert_key(index));
    }
    inline constexpr const_reference at(user_key_type index) const {
        return container().at(convert_key(index));
    }
    inline constexpr reference operator[](user_key_type index) {
        return container().operator[](convert_key(index));
    }
    inline constexpr const_reference operator[](user_key_type index) const {
        return container().operator[](convert_key(index));
    }
    inline constexpr user_key_type begin_index() const {
        return _this()->converter().monotonic_revert(0);
    }
    inline constexpr user_key_type end_index() const {
        return _this()->converter().monotonic_revert(container().size());
    }
};

template <typename _Impl, typename _ContainerType, typename _ConverterType>
class resubscript_base
        : public _resubscript_base<_Impl, _ContainerType, _ConverterType> {
    using _base = _resubscript_base<_Impl, _ContainerType, _ConverterType>;

   public:
    using container_type = _ContainerType;
    using converter_type = _ConverterType;
    using _base::_base;
    // data fields
    converter_type _converter;
    container_type _container;
    // end of data fields
    template <typename... Args>
    constexpr resubscript_base(converter_type converter, Args... arg)
            : _converter(converter), _container(arg...) {}

    resubscript_base& operator=(resubscript_base&&) = default;
};

template <typename _Impl, typename _ContainerType, typename _ConverterType>
class resubscript_base<_Impl, _ContainerType&, _ConverterType>
        : public _resubscript_base<_Impl, _ContainerType, _ConverterType> {
    using _base = _resubscript_base<_Impl, _ContainerType, _ConverterType>;

   public:
    using container_type = _ContainerType&;
    using converter_type = _ConverterType;
    using _base::_base;
    // data fields
    converter_type _converter;
    container_type _container;
    // end of data fields
    constexpr resubscript_base(
            converter_type converter, container_type& container) noexcept
            : _converter(converter), _container(container) {}
    resubscript_base& operator=(const resubscript_base&) = default;
};

template <typename _Impl, typename _ContainerType, typename _ConverterType>
class reindex_base
        : public resubscript_base<_Impl, _ContainerType, _ConverterType> {
   protected:
    using _base = resubscript_base<_Impl, _ContainerType, _ConverterType>;
    static_assert(std::is_integral<typename resubscript_base<
                          reindex_base<_Impl, _ContainerType, _ConverterType>,
                          _ContainerType, _ConverterType>::key_type>::value);

   public:
    using typename _base::container_type;
    using typename _base::converter_type;
    using iterator = typename _base::iterator;
    using index_type = typename _base::key_type;
    using user_index_type = typename _base::user_key_type;
    using value_type = typename _base::value_type;
    static_assert(std::is_same<index_type, typename _base::size_type>::value);

    using _base::_base;

    template <class... Args>
    inline iterator emplace(user_index_type user_pos, Args&&... args) {
        auto& container = this->container();
        return container.emplace(
                container.begin() + this->convert_key(user_pos), args...);
    }
    inline iterator insert(user_index_type user_pos, const value_type& value) {
        auto& container = this->container();
        return container.insert(
                container.begin() + this->convert_key(user_pos), value);
    }
    inline iterator insert(user_index_type user_pos, value_type&& value) {
        auto& container = this->container();
        return container.insert(
                container.begin() + this->convert_key(user_pos),
                std::move(value));
    }
    template <class InputIt>
    inline iterator insert(
            user_index_type user_pos, InputIt first, InputIt last) {
        auto& container = this->container();
        return container.insert(
                container.begin() + this->convert_key(user_pos), first, last);
    }
    inline iterator insert(
            user_index_type user_pos, std::initializer_list<value_type> ilist) {
        auto& container = this->container();
        return container.insert(
                container.begin() + this->convert_key(user_pos), ilist);
    }
    inline iterator erase(user_index_type user_pos) {
        auto& container = this->container();
        return container.erase(container.begin() + this->convert_key(user_pos));
    }
    inline iterator erase(
            user_index_type user_first, user_index_type user_last) {
        auto first = this->convert_key(user_first);
        auto last = this->convert_key(user_last);
        auto& container = this->container();
        const auto begin = container.begin();
        return container().erase(begin + first, begin + last);
    }
};

template <typename _base>
class bijective_mixin {
    class _public_impl : public _base::type {
       public:
        using _base::type::_container;
        using _base::type::_convert;
        using _base::type::_reverse_convert_key;
    };

   protected:
    inline constexpr const auto* _this() const noexcept {
        return static_cast<const _public_impl*>(this);
    }
    inline constexpr auto* _this() noexcept {
        return static_cast<_public_impl*>(this);
    }
};

template <typename _Impl, typename _ContainerType, typename _ConverterType>
class remap_base
        : public resubscript_base<_Impl, _ContainerType, _ConverterType> {
    using _base = resubscript_base<
            remap_base<_Impl, _ContainerType, _ConverterType>, _ContainerType,
            _ConverterType>;
    static_assert(std::is_same<
                  typename _base::container_type::key_type,
                  typename _base::key_type>::value);

   public:
    using size_type = typename _base::size_type;
    using user_key_type = typename _base::user_key_type;
    inline size_type erase(const user_key_type& user_key) {
        return this->container().erase(this->_this()->_convert(user_key));
    }
};

template <
        typename _T, typename _ContainerType = std::vector<_T>,
        typename _ConverterType = rebase_converter<>>
class rebase : public reindex_base<
                       rebase<_T, _ContainerType, _ConverterType>,
                       _ContainerType, _ConverterType> {
    using _base = reindex_base<
            rebase<_T, _ContainerType, _ConverterType>, _ContainerType,
            _ConverterType>;

   public:
    using size_type = typename _base::size_type;
    using key_type = typename _base::key_type;
    using user_key_type = typename _base::user_key_type;
    using container_type = typename _base::container_type;

    using _base::_base;  // activate constructor
};

template <
        typename _T, typename _ContainerType = std::vector<_T>,
        typename _ConverterType = slit_converter<>>
class slit : public reindex_base<
                     slit<_T, _ContainerType, _ConverterType>, _ContainerType,
                     _ConverterType> {
    using _base = reindex_base<
            slit<_T, _ContainerType, _ConverterType>, _ContainerType,
            _ConverterType>;

   public:
    using container_type = typename _base::container_type;
    using converter_type = _ConverterType;
    using size_type = typename _base::size_type;
    using key_type = typename converter_type::key_type;
    using user_key_type = typename converter_type::user_key_type;

    using _base::_base;  // activate constructor
};
static_assert(std::is_same<slit<int>::type, slit<int>>::value);

}  // namespace reindex

#endif
