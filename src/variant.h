#pragma once

#include "variant_base.h"

namespace platform {

template <class Platform, class ValueType>
class variant : public variant_base {
 public:
  using platform = Platform;
  using value_type = ValueType;
  using allocator = typename platform::allocator<value_type>;

  variant(const arguments_map& args)
      : variant_base(args),
        m_src_data(storage_size()),
        m_dst_data(storage_size()) {}

 protected:
  value_type *m_src, *m_dst;

  value_type* src_data() { return m_src_data.data(); }
  value_type* dst_data() { return m_dst_data.data(); }

 private:
  const value_type& src(int i, int j, int k) const {
    return m_src_data[zero_offset() + index(i, j, k)];
  }

  const value_type& dst(int i, int j, int k) const {
    return m_dst_data[zero_offset() + index(i, j, k)];
  }

  bool verify(const std::string& kernel) const override {
    if (kernel == "copy")
      return verify_loop(
          [&](int i, int j, int k) { return dst(i, j, k) == src(i, j, k); });
    throw std::logic_error("Error: unknown stencil '" + kernel + "'");
  }

  std::size_t bytes(const std::string& kernel) const override {
    if (kernel == "copy")
      return sizeof(value_type) * 2 * isize() * jsize() * ksize();
    throw std::logic_error("Error: unknown stencil '" + kernel + "'");
  }

  std::vector<value_type, allocator> m_src_data, m_dst_data;
};

}  // platform
