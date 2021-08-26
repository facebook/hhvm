/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/facts/file-facts.h"

template <typename T> Array makeVec(const std::vector<T>& types) {
  VecInit ret{types.size()};
  for (auto const& type : types) {
    ret.append(type);
  }
  return ret.toArray();
}

#define X(str) const StringData* s_##str = makeStaticString(#str);
// MethodDetails
X(name)
X(attributes)

// TypeDetails
X(kind)
X(flags)
X(baseTypes)
X(requireExtends)
X(requireImplements)
X(methods)

// FileFacts
X(types)
X(functions)
X(constants)
X(sha1sum)
#undef X

Array makeDict(const MethodDetails& method) {
  return make_dict_array(
      s_name, method.m_name, s_attributes, makeVec(method.m_attributes));
}

Array makeDict(const TypeDetails& type) {
#define X(str) s_##str, type.m_##str
  return make_dict_array(
      X(name),
      X(kind),
      X(flags),
      X(baseTypes),
      X(attributes),
      X(requireExtends),
      X(requireImplements),
      X(methods));
#undef X
}

template <> Array makeVec(const std::vector<TypeDetails>& types) {
  VecInit ret{types.size()};
  for (auto const& type : types) {
    ret.append(makeDict(type));
  }
  return ret.toArray();
}

Array makeDict(const FileFacts& facts) {
  return make_dict_array(
      s_types,
      makeVec(facts.m_types),
      s_functions,
      makeVec(facts.m_functions),
      s_constants,
      makeVec(facts.m_constants),
      s_attributes,
      makeVec(facts.m_attributes),
      s_sha1sum,
      String{facts.m_sha1hex});
}
