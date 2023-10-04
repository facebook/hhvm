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

#include "hphp/tools/debug-parser/debug-parser.h"

#include "hphp/util/assertions.h"

#include <folly/Format.h>

namespace debug_parser {

////////////////////////////////////////////////////////////////////////////////

VoidType Type::s_void_type;

const char* show(ObjectTypeName::Linkage linkage) {
  switch (linkage) {
    case ObjectTypeName::Linkage::external: return "external";
    case ObjectTypeName::Linkage::internal: return "internal";
    case ObjectTypeName::Linkage::none:     return "none";
    case ObjectTypeName::Linkage::pseudo:   return "pseudo";
  }
  not_reached();
}

Type::~Type() {
  // Normally a Type can't have an empty DiscriminatedPtr, but it can happen
  // with the move constructor (since we use swap on a default constructed
  // DiscriminatedPtr), so don't try to run match if we're empty.
  if (m_type.empty()) return;

  match<void>(
    [](const VoidType*) { /* Using static instance */ },
    [](const FuncType* t) { delete t; },
    [](const PtrType* t) { delete t; },
    [](const RefType* t) { delete t; },
    [](const RValueRefType* t) { delete t; },
    [](const ArrType* t) { delete t; },
    [](const ObjectType* t) { delete t; },
    [](const MemberType* t) { delete t; },
    [](const ConstType* t) { delete t; },
    [](const VolatileType* t) { delete t; },
    [](const RestrictType* t) { delete t; }
  );
}

std::string Type::toString() const {
  // Functions need some special handling when printing, since the syntax for
  // function and pointer to function is slightly different. In fact, "function"
  // on its own isn't an actual type in C or C++, but its easier for everything
  // else to have them be separate concepts.
  const auto handle_func = [](const FuncType& func, bool add_ptr) {
    std::string base = add_ptr ?
      folly::sformat("{}(*)", func.ret.toString()) :
      func.ret.toString();

    std::string args = "(";
    for (std::size_t i = 0; i < func.args.size(); ++i) {
      args = (i != func.args.size()-1) ?
        folly::sformat("{}{},", args, func.args[i].toString()) :
        folly::sformat("{}{})", args, func.args[i].toString());
    }
    return base + args;
  };

  return match<std::string>(
    [&](const VoidType*) -> std::string { return "void"; },
    [&](const FuncType* p) { return handle_func(*p, false); },
    [&](const PtrType* p) {
      if (auto func = p->pointee.asFunc()) {
        return handle_func(*func, true);
      } else {
        return folly::sformat("{}*", p->pointee.toString());
      }
    },
    [&](const RefType* p) {
      return folly::sformat("{}&", p->referenced.toString());
    },
    [&](const RValueRefType* p) {
      return folly::sformat("{}&&", p->referenced.toString());
    },
    [&](const ArrType* p) {
      return p->count ?
        folly::sformat("{}[{}]", p->element.toString(), *p->count) :
        folly::sformat("{}[]", p->element.toString());
    },
    [&](const MemberType* p) {
      return folly::sformat("{} {}::", p->member.toString(), p->obj.name.name);
    },
    [&](const ObjectType* p) { return p->name.name; },
    [&](const ConstType* p) {
      return folly::sformat("{} const", p->modified.toString());
    },
    [&](const VolatileType* p) {
      return folly::sformat("{} volatile", p->modified.toString());
    },
    [&](const RestrictType* p) {
      return folly::sformat("{} restrict", p->modified.toString());
    }
  );
}

/*
 * Factory functions. Do the simplest here thing here and just use ifdefs to
 * control which implementation gets returned.
 */

std::unique_ptr<TypeParser> TypeParser::make(const std::string& filename,
                                             int num_threads) {
#if defined(__linux__) || defined(__FreeBSD__)
  std::unique_ptr<TypeParser> make_dwarf_type_parser(const std::string&,
                                                     int);
  return make_dwarf_type_parser(filename, num_threads);
#else
  return nullptr;
#endif
}

std::unique_ptr<Printer> Printer::make(const std::string& filename) {
#if defined(__linux__) || defined(__FreeBSD__)
  std::unique_ptr<Printer> make_dwarf_printer(const std::string&);
  return make_dwarf_printer(filename);
#else
  return nullptr;
#endif
}

////////////////////////////////////////////////////////////////////////////////

}
