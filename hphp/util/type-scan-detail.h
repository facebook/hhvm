/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_UTIL_TYPE_SCAN_DETAIL_H_
#define incl_HPHP_UTIL_TYPE_SCAN_DETAIL_H_

#include <array>
#include <tuple>

/*
 * Here's how all the type scanner machinery works at a high level:
 *
 * - All "countable" types (basically ones with explicitly managed ref-counts)
 *   have MarkCollectable<> instantiations.
 *
 * - Any call to getIndexForMalloc<T> or getIndexForScan<T> present in the
 *   source results in an Indexer<> instantiation, which provides a static
 *   storage location for the type index. The type index is assigned
 *   "kIndexUnknown" at start-up (conveniently this has value 0, so they can be
 *   stored in the .bss).
 *
 * - The type annotation macros spit out various static fields and member
 *   functions with special names. These static fields may have special types
 *   which are template instantiations containing other types.
 *
 * - The scanner generator is run during the build, after the final link where
 *   the executable is produced. It parses the debug information inside the
 *   final executable, and sees all the MarkCollectable<T> and Indexer<T>
 *   instantiations. Using these instantiations, as well as the presence of the
 *   specially named fields and member functions, the scanner generator
 *   generates scanning functions. Every Indexer<T> instantiation is assigned a
 *   type-index and a scanner function.
 *
 * - These generated functions, along with metadata to map the type-index to the
 *   function, is emitted as C++ code and compiled into a shared object. The
 *   shared object includes an initialization function which sets all the static
 *   Indexer<T>::s_index instances to the proper type-index. This can be done
 *   because the fixed address of the static instances can be found in the debug
 *   information.
 *
 * - Since the final executable has already been produced, the shared object is
 *   embedded into the executable in a custom section. The reason for the shared
 *   object and loading at start-up is due to build system limitations. In
 *   addition, generating the scanner functions after the final executable is
 *   produced provides certain advantages (such as having the final relocated
 *   addresses available).
 *
 * - Sometime at start-up, the init() function is called. This function finds
 *   the shared object in the custom section, loads it dynamically, and calls
 *   the initialization function within it. This function, as described above,
 *   initializes all the type indices to their generated values.
 *
 * - The memory manager calls getIndexForMalloc<T> for all allocations, and
 *   stores the returned type-index somewhere it can be retrieved later.
 *
 * - The garbage collector instantiates Scanner, calls Scanner::scan() for
 *   roots, and Scanner::scanByIndex() for values on the request heap (using the
 *   stored type-index). The Scanner is populated with pointers found by the
 *   scanners (both automatically generated and custom). The GC uses these
 *   pointers to further populate the worklist.
 */

namespace HPHP { namespace type_scan {

////////////////////////////////////////////////////////////////////////////////

struct Scanner;

namespace detail {

////////////////////////////////////////////////////////////////////////////////

// A special action used internally to designate that this type index does not
// represent a type being allocated on the heap. Instead, the type index will
// just be used for scanning. This is useful because we won't consider pointers
// to such types as being "interesting" (because they can't be sitting in the
// request heap).
struct ScanAction {};

/*
 * Instantiations of Indexer<> is the signal to the scanner generator to
 * allocate a type-index for that particular type and to generate a matching
 * scanner function.
 *
 * The static member "s_index" stores the assigned type-index corresponding to
 * this Indexer<>. At start-up, all Indexer<>::s_index instantiations default to
 * kIndexUnknown, but when the type scanning infrastructure is initialized, the
 * generated code will overwrite all the s_index instances with their proper
 * type-indices. (It can do this because it knows the addresses of all the
 * instances from the debug information).
 */
template <typename T, typename A> struct Indexer {
  static volatile Index
    ATTRIBUTE_USED ATTRIBUTE_UNUSED EXTERNALLY_VISIBLE s_index;
  // This is never used, but Clang needs it or it emits *no* debug information
  // about A.
  A m_action;
};

template <typename T, typename A>
  volatile Index Indexer<T, A>::s_index = kIndexUnknown;

// Empty types used as part of type annotations. These types don't really
// matter, as the behavior is inferred from the field names.
struct IgnoreField {};
struct ConservativeField {};
struct FlexibleArrayField {};
template <typename... T> struct IgnoreBase {};
template <typename... T> struct CustomBase {};
template <typename... T> struct Custom {};
template <typename... T> struct SilenceForbiddenBase {};

// Template metaprogramming to determine statically if a type is
// uninteresting. This is mainly primitive types (like int), and pointers or
// references to such. Also includes some overloads to handle types of such like
// pair, tuple, or array.

template <typename T, typename = void>
struct UninterestingImpl : std::false_type {};

template <typename T>
struct UninterestingImpl<
  T,
  typename std::enable_if<std::is_pointer<T>::value>::type
> : std::conditional<
      std::is_void<typename std::remove_pointer<T>::type>::value,
      std::false_type,
      UninterestingImpl<
        typename std::remove_cv<typename std::remove_pointer<T>::type>::type
      >
    >::type {};

template <typename T>
struct UninterestingImpl<
  T,
  typename std::enable_if<std::is_reference<T>::value>::type
> : UninterestingImpl<
      typename std::remove_cv<typename std::remove_reference<T>::type>::type
    > {};

template <typename T>
struct UninterestingImpl<
  T,
  typename std::enable_if<std::is_array<T>::value>::type
> : UninterestingImpl<
      typename std::remove_cv<typename std::remove_all_extents<T>::type>::type
    > {};

template <typename T, std::size_t N>
struct UninterestingImpl<std::array<T, N>> :
      UninterestingImpl<typename std::remove_cv<T>::type> {};

template <typename T, typename U>
struct UninterestingImpl<std::pair<T, U>> : std::conditional<
  UninterestingImpl<typename std::remove_cv<T>::type>::value,
  UninterestingImpl<typename std::remove_cv<U>::type>,
  std::false_type
>::type {};

template <typename T, typename... U>
struct UninterestingImpl<std::tuple<T, U...>> : std::conditional<
  UninterestingImpl<typename std::remove_cv<T>::type>::value,
  UninterestingImpl<std::tuple<typename std::remove_cv<U>::type...>>,
  std::false_type
> {};
template <> struct UninterestingImpl<std::tuple<>> : std::true_type {};

template <typename T>
struct UninterestingImpl<
  T,
  typename std::enable_if<
    std::is_fundamental<T>::value ||
    std::is_enum<T>::value ||
    std::is_member_pointer<T>::value ||
    std::is_function<T>::value
    >::type
> : std::true_type {};

template <typename T> using Uninteresting =
  UninterestingImpl<typename std::remove_cv<T>::type>;

// Template metaprogramming to determine if a type is some kind of pointer to
// void. IE, void*, void**, etc, etc, as well as some overloads to handle types
// like pair/tuple/array of such.
template <typename T, typename = void>
struct IsVoidImpl : std::false_type {};

template <typename T>
struct IsVoidImpl <
  T,
  typename std::enable_if<std::is_pointer<T>::value>::type
> : std::conditional<
      std::is_void<typename std::remove_pointer<T>::type>::value,
      std::true_type,
      IsVoidImpl<
        typename std::remove_cv<typename std::remove_pointer<T>::type>::type
      >
    >::type {};

template <typename T>
struct IsVoidImpl<
  T,
  typename std::enable_if<std::is_reference<T>::value>::type
> : IsVoidImpl<
      typename std::remove_cv<typename std::remove_reference<T>::type>::type
    > {};

template <typename T>
struct IsVoidImpl<
  T,
  typename std::enable_if<std::is_array<T>::value>::type
> : IsVoidImpl<
      typename std::remove_cv<typename std::remove_all_extents<T>::type>::type
    > {};

template <typename T, std::size_t N>
struct IsVoidImpl<std::array<T, N>> :
      IsVoidImpl<typename std::remove_cv<T>::type> {};

template <typename T, typename U>
struct IsVoidImpl<std::pair<T, U>> : std::conditional<
  IsVoidImpl<typename std::remove_cv<T>::type>::value,
  std::true_type,
  IsVoidImpl<typename std::remove_cv<U>::type>
>::type {};

template <typename T, typename... U>
struct IsVoidImpl<std::tuple<T, U...>> : std::conditional<
  IsVoidImpl<typename std::remove_cv<T>::type>::value,
  std::true_type,
  IsVoidImpl<std::tuple<typename std::remove_cv<U>::type...>>
> {};
template <> struct IsVoidImpl<std::tuple<>> : std::false_type {};

template <typename T> using IsVoid =
  IsVoidImpl<typename std::remove_cv<T>::type>;

// Template metaprogramming to check for unbounded arrays (arrays with no
// specified size).
template <typename T> struct UnboundedArrayImpl : std::false_type {};
template <typename T> struct UnboundedArrayImpl<T[]> : std::true_type {};
// ??? GCC considers a flexible array member as an array of size 0 (not
// unbounded).
template <typename T> struct UnboundedArrayImpl<T[0]> : std::true_type {};

template <typename T> using UnboundedArray =
  UnboundedArrayImpl<typename std::remove_cv<T>::type>;

// Table of type names and scanner function pointers indexed by type-index. At
// start-up, this table will just have two fixed entries (for the two "unknown"
// type-indices), but initializating the type scanning machinery will replace it
// with the proper table.
struct Metadata {
  const char* const m_name;
  void (*m_scan)(Scanner&, const void*, std::size_t);
};
extern const Metadata* g_metadata_table;
extern std::size_t g_metadata_table_size;

// Check if a type with the given name is on the list of explicitly ignored
// types. Certain types might give the scanner generator problems, but we can't
// add annotations because we do not control its definition (IE, a third-party
// type). These types usually aren't relevant to GC, so a list of types to
// explicitly ignore is maintained. The template portion (if any) of the type
// name is ignored.
bool isIgnoredType(const std::string& name);

// Check if a type with the given name is on the list of template types which
// cannot contain request heap allocated types. This is mainly for templates
// (like the standard containers) where we have request heap aware versions and
// is to prevent people accidently using the standard kind.
bool isForbiddenTemplate(const std::string& name);

// Check if a type with the given name is on the list of template types which
// will always be scanned conservatively.
bool isForcedConservativeTemplate(const std::string& name);

// Macro trickery to generate field names for type annotations.
#define TYPE_SCAN_BUILD_NAME(A,B) TYPE_SCAN_BUILD_NAME_HIDDEN(A,B)
#define TYPE_SCAN_BUILD_NAME_HIDDEN(A,B) A##B##_

#define TYPE_SCAN_CUSTOM_GUARD_NAME _type_scan_custom_guard_
#define TYPE_SCAN_CUSTOM_NAME _type_scan_custom_
#define TYPE_SCAN_CUSTOM_FIELD_NAME _type_scan_custom_field_
#define TYPE_SCAN_CUSTOM_BASES_NAME _type_scan_custom_bases_
#define TYPE_SCAN_CUSTOM_BASES_SCANNER_NAME _type_scan_custom_bases_scanner_
#define TYPE_SCAN_IGNORE_NAME _type_scan_ignore_
#define TYPE_SCAN_IGNORE_FIELD_NAME _type_scan_ignore_field_
#define TYPE_SCAN_IGNORE_BASE_NAME _type_scan_ignore_base_
#define TYPE_SCAN_CONSERVATIVE_NAME _type_scan_conservative_
#define TYPE_SCAN_CONSERVATIVE_FIELD_NAME _type_scan_conservative_field_
#define TYPE_SCAN_FLEXIBLE_ARRAY_FIELD_NAME _type_scan_flexible_array_field_
#define TYPE_SCAN_SILENCE_FORBIDDEN_BASE_NAME _type_scan_silence_forbidden_base_

constexpr const char* const kInitFuncName = "hphp_type_scan_module_init";

#define TYPE_SCAN_STRINGIFY(X) TYPE_SCAN_STRINGIFY_HIDDEN(X)
#define TYPE_SCAN_STRINGIFY_HIDDEN(X) #X

// Store all the special field names which act as type annotations as static
// constants. This lets us the scanner generator use them while ignoring the
// macro versions.
constexpr const char* const kCustomGuardName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CUSTOM_GUARD_NAME);
constexpr const char* const kCustomName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CUSTOM_NAME);
constexpr const char* const kCustomFieldName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CUSTOM_FIELD_NAME);
constexpr const char* const kCustomBasesName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CUSTOM_BASES_NAME);
constexpr const char* const kCustomBasesScannerName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CUSTOM_BASES_SCANNER_NAME);
constexpr const char* const kIgnoreName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_IGNORE_NAME);
constexpr const char* const kIgnoreFieldName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_IGNORE_FIELD_NAME);
constexpr const char* const kIgnoreBaseName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_IGNORE_BASE_NAME);
constexpr const char* const kConservativeName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CONSERVATIVE_NAME);
constexpr const char* const kConservativeFieldName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_CONSERVATIVE_FIELD_NAME);
constexpr const char* const kFlexibleArrayFieldName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_FLEXIBLE_ARRAY_FIELD_NAME);
constexpr const char* const kSilenceForbiddenBaseName =
  TYPE_SCAN_STRINGIFY(TYPE_SCAN_SILENCE_FORBIDDEN_BASE_NAME);

#undef TYPE_SCAN_STRINGIFY_HIDDEN
#undef TYPE_SCAN_STRINGIFY

////////////////////////////////////////////////////////////////////////////////

}}}

#endif
