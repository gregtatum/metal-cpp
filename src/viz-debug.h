#pragma once
#include "iostream"
#include "viz-math.h"

namespace viz {

static void
DebugIndent(size_t tabDepth)
{
  for (size_t i = 0; i < tabDepth; i++) {
    std::cout << "  ";
  }
}

template<typename T>
struct debug
{
  static void print(T const& v) { std::cout << v; }
};

template<>
struct debug<Vector2>
{
  static void print(Vector2 const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << "]";
  }
};

template<>
struct debug<Vector3>
{
  static void print(Vector3 const& v, size_t tabDepth = 0)
  {
    DebugIndent(tabDepth);
    std::cout << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
  }
};

template<typename T>
struct debug<std::vector<T>>
{
  static void print(std::vector<T> const& v, size_t tabDepth = 0)
  {
    if (v.size() == 0) {
      std::cout << "std::vector{ empty }";
      return;
    }

    std::cout << "std::vector{\n";
    for (size_t i = 0; i < v.size(); i++) {
      if (i != 0) {
        std::cout << ",\n";
      }

      if (tabDepth != 0 && i > 1000) {
        DebugIndent(tabDepth + 1);
        std::cout << "(..." << v.size() - i << " more...)\n";
        break;
      }
      debug<T>::print(v[i], tabDepth + 1);
    }

    DebugIndent(tabDepth);
    std::cout << "}";
  }
};

// namespace traits {
// template<typename P>
// struct data_type
// {};

// struct pod
// {};

// // clang-format off
// template <>
// struct data_type<uint32_t> { typedef pod type; };
// template <>
// struct data_type<float> { typedef pod type; };

// // clang-format on

// } // tags

} // viz
