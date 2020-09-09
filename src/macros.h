#define VIZ_DEBUG_OBJ_PROP(obj, tabDepth, property, propertyType)              \
  DebugIndent(tabDepth + 1);                                                   \
  std::cout << #property << ": ";                                              \
  ::viz::DebugWithoutNewline<propertyType>(obj.property, tabDepth + 1);

#define VIZ_DEBUG_OBJ_HEADER(objType, obj, tabDepth)                           \
  DebugIndent(tabDepth);                                                       \
  std::cout << "Mesh {\n";

#define VIZ_DEBUG_OBJ_FOOTER(tabDepth)                                         \
  std::cout << ",\n";                                                          \
  DebugIndent(tabDepth);                                                       \
  std::cout << "}";

// This macro should be used inside the viz namespace. It will define the
// necessary tagged dispatch structures to be able to provide debug printing of
// a struct.
#define VIZ_DEBUG_OBJ(traitName, ClassName, applyImpl)                         \
  namespace traits {                                                           \
  struct traitName                                                             \
  {};                                                                          \
  template<>                                                                   \
  struct tag<ClassName>                                                        \
  {                                                                            \
    typedef mesh type;                                                         \
    typedef empty child;                                                       \
  };                                                                           \
  }                                                                            \
                                                                               \
  namespace dispatch {                                                         \
  template<>                                                                   \
  struct debug<traits::traitName, ClassName>                                   \
  {                                                                            \
    static void apply(ClassName const& traitName,                              \
                      size_t tabDepth = 0) applyImpl                           \
  };                                                                           \
  }
