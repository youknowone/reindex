
#include <array>
#include <deque>
#include <unordered_map>
#include <vector>

#include <reindex/reindex.hh>

#include <gtest/gtest.h>

// notable examples are commented with **Example**

namespace {

template <typename _T, typename _ContainerType>
class testable_slit : public reindex::slit<_T, _ContainerType> {
  using slit = reindex::slit<_T, _ContainerType>;

 public:
  using slit::_container;
  using slit::_convert;
  using slit::_step;
};

struct X {
  int a;
  int b;
};

TEST(reindex, slit2) {
  using container_type = std::unordered_map<std::string, char>;
  auto table = reindex::rebase<container_type>(100);
}

template <typename T>
void _test_slit(T& slit) {
  // **Example** reindex::slit behaviors
  assert(slit.begin_index() == 1000);
  assert(slit.end_index() == 3500);
  slit.at(1000) = {10, 20};
  slit[2000] = {20, 30};
  assert(slit.at(1000).a == 10);
  assert(slit[1000].b == 20);
  assert(slit.at(2000).b == 30);

  auto& vslit = *reinterpret_cast<testable_slit<
      typename T::bare_container_type::value_type,
      typename T::container_type>*>(&slit);
  assert(&vslit.at(1000) == &slit.at(1000));

  assert(vslit._convert(1000) == 0);
  assert(vslit._convert(1005) == 1);
  assert(vslit._convert(2000) == (2000 - 1000) / 5);

  auto& x1 = slit[1000];
  auto& x2 = slit[2000];
  slit.reindex(2000, 10);
  assert(&x1 == &slit[2000]);
  assert(&x2 != &slit[2000]);
  slit.reindex(1000, 5);
}

TEST(reindex, slit_containers) {
  // **Example** slit for various containers
  {
    // default: slit for std::vector<X>
    auto slit = reindex::slit<X>(1000, 5, 500);
    _test_slit(slit);
    slit.emplace(1020, X{12, 24});
    assert(slit[1020].a == 12);
    slit.insert(1030, X{13, 26});
    assert(slit[1030].b == 26);
    assert(slit[1025].b != 26);
    slit.erase(1020);
    assert(slit[1025].b == 26);
  }
  {
    // **Example** specify a container type not to use default container
    // using array
    auto slit = reindex::slit<X, std::array<X, 500>>(1000, 5);
    _test_slit(slit);
  }
  {
    // **Example** slit for std::deque<X>
    auto slit = reindex::slit<X, std::deque<X>>(1000, 5, 500);
    _test_slit(slit);
  }
}

TEST(reindex, slit_refs) {
  // **Example** slit for included storage vs referenced storage
  using container_type = std::vector<X>;
  {
    auto slit = reindex::slit<X>(1000, 5);
    slit.container().resize(500);
    _test_slit(slit);
  }
  {
    container_type container(500);
    auto slit1 = reindex::slit<X, container_type&>(1000, 5, container);
    _test_slit(slit1);
    auto slit2 =
        reindex::slit<X, container_type>(1000, 5, std::move(container));
    _test_slit(slit2);
  }
}

template <typename T>
void _test_rebase(T& rebase) {
  assert(rebase.begin_index() == 1000);
  assert(rebase.end_index() == 1500);

  rebase.at(1000) = {10, 20};
  rebase[1100] = {20, 30};
  assert(rebase.at(1000).a == 10);
  assert(rebase[1100].b == 30);
}

TEST(reindex, rebase) {
  using container_type = std::array<X, 500>;
  {
    auto rebase = reindex::rebase<X, container_type>(1000);
    _test_rebase(rebase);
  }
  {
    container_type container;
    auto rebase1 = reindex::rebase<X, container_type&>(1000, container);
    _test_rebase(rebase1);
    auto rebase2 =
        reindex::rebase<X, container_type>(1000, std::move(container));
    _test_rebase(rebase2);
  }
}

}  // namespace
