#pragma once
#include "iostream"
#include "viz/math.h"

namespace viz {

namespace traits {

// clang-format off
struct empty {};
struct pod {};
struct tuple2 {};
struct tuple3 {};
struct tuple4 {};
struct float3x3 {};
struct float4x4 {};
struct list {};
// clang-format on

template<typename Child>
struct tag
{};

template<>
struct tag<uint32_t>
{
  typedef pod type;
  typedef empty child;
};
template<>
struct tag<float>
{
  typedef pod type;
  typedef empty child;
};
template<>
struct tag<Vector3>
{
  typedef tuple3 type;
  typedef empty child;
};
template<>
struct tag<simd::float3>
{
  typedef tuple3 type;
  typedef empty child;
};
template<>
struct tag<simd::float4>
{
  typedef tuple4 type;
  typedef empty child;
};
template<typename Child>
struct tag<std::array<Child, 3>>
{
  typedef tuple3 type;
  typedef Child child;
};
template<>
struct tag<Vector2>
{
  typedef tuple2 type;
  typedef empty child;
};
template<>
struct tag<simd::float3x3>
{
  typedef float3x3 type;
  typedef simd::float3 child;
};
template<>
struct tag<simd::float4x4>
{
  typedef float4x4 type;
  typedef simd::float4 child;
};
template<typename Child>
struct tag<std::vector<Child>>
{
  typedef list type;
  typedef Child child;
};
template<typename Child>
struct tag<std::span<Child>>
{
  typedef list type;
  typedef Child child;
};

} // traits

namespace dispatch {
static void
DebugIndent(size_t tabDepth)
{
  for (size_t i = 0; i < tabDepth; i++) {
    std::cout << "  ";
  }
}

template<typename Tag, typename T>
struct debug
{
  static void apply(T const& v, size_t tabDepth = 0) { std::cout << v; }
};

template<typename T>
struct debug<traits::tuple2, T>
{
  static void apply(T const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << "]";
  }
};

template<typename T>
struct debug<traits::tuple3, T>
{
  static void apply(T const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
  }
};

template<typename T>
struct debug<traits::tuple4, T>
{
  static void apply(T const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3]
              << "]";
  }
};

template<typename T>
struct debug<traits::float3x3, T>
{
  static void apply(T const& m, size_t tabDepth = 0)
  {
    using Child = typename traits::tag<T>::child;
    using ChildType = typename traits::tag<Child>::type;

    std::cout << "[" << std::endl;

    debug<traits::tuple3, simd::float3>::apply(m.columns[0], tabDepth + 1);
    std::cout << "," << std::endl;
    debug<traits::tuple3, simd::float3>::apply(m.columns[1], tabDepth + 1);
    std::cout << "," << std::endl;
    debug<traits::tuple3, simd::float3>::apply(m.columns[2], tabDepth + 1);
    std::cout << "," << std::endl;

    DebugIndent(tabDepth);
    std::cout << "]";
  }
};

template<typename T>
struct debug<traits::float4x4, T>
{
  static void apply(T const& m, size_t tabDepth = 0)
  {
    using Child = typename traits::tag<T>::child;
    using ChildType = typename traits::tag<Child>::type;

    std::cout << "[" << std::endl;

    debug<traits::tuple4, simd::float4>::apply(m.columns[0], tabDepth + 1);
    std::cout << "," << std::endl;
    debug<traits::tuple4, simd::float4>::apply(m.columns[1], tabDepth + 1);
    std::cout << "," << std::endl;
    debug<traits::tuple4, simd::float4>::apply(m.columns[2], tabDepth + 1);
    std::cout << "," << std::endl;
    debug<traits::tuple4, simd::float4>::apply(m.columns[2], tabDepth + 1);
    std::cout << "," << std::endl;

    DebugIndent(tabDepth);
    std::cout << "]";
  }
};

template<typename T>
struct debug<traits::list, T>
{
  using Child = typename traits::tag<T>::child;
  using ChildType = typename traits::tag<Child>::type;

  static void apply(T const& v, size_t tabDepth = 0)
  {
    if (v.size() == 0) {
      std::cout << "{ empty }";
      return;
    }

    std::cout << "{\n";
    for (size_t i = 0; i < v.size(); i++) {
      if (i != 0) {
        std::cout << ",\n";
      }

      if (tabDepth != 0 && i > 1000) {
        DebugIndent(tabDepth + 1);
        std::cout << "(..." << v.size() - i << " more...)\n";
        break;
      }

      debug<ChildType, Child>::apply(v[i], tabDepth + 1);
    }
    std::cout << "\n";

    DebugIndent(tabDepth);
    std::cout << "}";
  }
};
} // dispatch

template<typename T>
void
Debug(T const& v, size_t tabDepth = 0)
{
  dispatch::debug<typename traits::tag<T>::type, T>::apply(v, tabDepth);
  std::cout << "\n";
}

/**
 * This variant ensures a final newline isn't added.
 */
template<typename T>
void
DebugWithoutNewline(T const& v, size_t tabDepth = 0)
{
  return dispatch::debug<typename traits::tag<T>::type, T>::apply(v, tabDepth);
}

} // viz
