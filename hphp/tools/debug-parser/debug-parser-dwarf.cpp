/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#if defined(__linux__) || defined(__FreeBSD__)

#include <folly/Demangle.h>
#include <folly/Format.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dwarf.h>
#include <libdwarf.h>

#include "hphp/util/assertions.h"

#include "hphp/tools/debug-parser/debug-parser.h"

/*
 * Debug parser for DWARF (using libdwarf)
 *
 * DWARF is structured as a forest of DIEs (Debug Information Entry). Each DIE
 * has a tag, which describes what kind of DIE it is, and a list of
 * attributes. Each attribute has a type, which identifies what it is, and a
 * value (the type of the value is implied by the attribute type). Furthermore,
 * a DIE can have other DIEs as children. The top-level DIEs correspond to
 * compilation-units, and all the children of these top-level DIEs correspond to
 * the information in that compilation-unit.
 *
 * The meaning and interpretation of the DIEs is deliberately left vague by the
 * standard, so different compilers can encode things in different ways (and no
 * implementation is bug free).
 */

namespace debug_parser { namespace {

////////////////////////////////////////////////////////////////////////////////

/*
 * libdwarf uses a very low-level janky C-style interface, so provide a simple
 * wrapper class to make some of the common operations easier.
 *
 * In a few cases, libdwarf keeps internal state, which forces you walk the DIEs
 * in a hierarchial manner. For this reason, many of the operations are
 * structured as for-each style iteration.
 */

struct DwarfState {
  explicit DwarfState(const std::string& filename);
  ~DwarfState();

  Dwarf_Half getTag(Dwarf_Die die);
  std::string tagToString(Dwarf_Half tag);
  std::string getDIEName(Dwarf_Die die);
  Dwarf_Off getDIEOffset(Dwarf_Die die);
  Dwarf_Half getAttributeType(Dwarf_Attribute attr);
  std::string attributeTypeToString(Dwarf_Half type);
  Dwarf_Half getAttributeForm(Dwarf_Attribute attr);
  std::string getAttributeValueString(Dwarf_Attribute attr);
  Dwarf_Bool getAttributeValueFlag(Dwarf_Attribute attr);
  Dwarf_Unsigned getAttributeValueUData(Dwarf_Attribute attr);
  Dwarf_Signed getAttributeValueSData(Dwarf_Attribute attr);
  Dwarf_Addr getAttributeValueAddr(Dwarf_Attribute attr);
  Dwarf_Off getAttributeValueRef(Dwarf_Attribute attr);
  Dwarf_Sig8 getAttributeValueSig8(Dwarf_Attribute attr);
  std::vector<Dwarf_Loc> getAttributeValueExprLoc(Dwarf_Attribute attr);

  template <typename F> void forEachChild(Dwarf_Die die, F&& f);
  template <typename F> void forEachAttribute(Dwarf_Die die, F&& f);
  template <typename F> void forEachCompilationUnit(F&& f);
  template <typename F> auto onDIEAtOffset(Dwarf_Off offset, F&& f) ->
    decltype(f(std::declval<Dwarf_Die>()));

  int fd;
  Dwarf_Debug dwarf;
  std::string filename;
};

/*
 * Fully qualified names aren't represented explicitly in DWARF. Instead the
 * structure of the DIEs mimics the nesting structure in the source (IE, a
 * nested class within a class nested within a namespace). So, in order to
 * infer the fully qualified name for any given class, the current scope is
 * tracked as the DIEs are walked.
 *
 * Likewise, DWARF as no concept of linkage, but the linkage is needed to know
 * which types are actually equivalent. Luckily, a type's linkage is closely
 * related to its scope (except for templates, see below), so it can be inferred
 * the same way.
 *
 * The scope is tracked as a stack of contexts, pushing and popping off contexts
 * when a namespace or type is entered or exited.
 */

struct Scope {
  explicit Scope(Dwarf_Off cu_offset)
    : m_cu_offset{cu_offset}
  {
    m_scope.emplace_back(
      ObjectTypeName{std::string{}, ObjectTypeName::Linkage::external},
      true
    );
  }

  Dwarf_Off cuOffset() const { return m_cu_offset; }

  ObjectTypeName name() const;

  ObjectTypeName::Linkage linkage() const {
    return m_scope.back().name.linkage;
  }

  std::size_t unnamedTypeCount() const {
    return m_scope.back().unnamed_count;
  }

  bool isInNamespaceScope() const {
    return m_scope.back().in_namespace_scope;
  }

  void incUnnamedTypeCount() { ++m_scope.back().unnamed_count; }

  folly::Optional<Dwarf_Off> typeOffset() const {
    return m_scope.back().offset;
  }

  void pushType(std::string name, Dwarf_Off offset) {
    m_scope.emplace_back(
      ObjectTypeName{std::move(name), linkage()},
      false
    );
    m_scope.back().offset = offset;
  }

  void pushAnonType(Dwarf_Off offset) {
    m_scope.emplace_back(
      ObjectTypeName{
        "(anonymous type)",
        ObjectTypeName::Linkage::none
      },
      false
    );
    m_scope.back().offset = offset;
  }

  void pushNamespace(std::string ns) {
    m_scope.emplace_back(
      ObjectTypeName{std::move(ns), linkage()},
      true
    );
  }

  void pushAnonNamespace() {
    m_scope.emplace_back(
      ObjectTypeName{
        "(anonymous namespace)",
        ObjectTypeName::Linkage::internal
      },
      true
    );
  }

  void pop() { m_scope.pop_back(); }

 private:
  struct Context {
    Context(ObjectTypeName name, bool in_namespace_scope)
        : name(std::move(name))
        , in_namespace_scope{in_namespace_scope} {}
    ObjectTypeName name;
    bool in_namespace_scope;
    std::size_t unnamed_count = 0;
    folly::Optional<Dwarf_Off> offset;
  };
  std::vector<Context> m_scope;
  Dwarf_Off m_cu_offset;

 public:
  static const std::string s_pseudo_type_name;
};

/*
 * Actual implementation of TypeParser for DWARF.
 */

struct TypeParserImpl : TypeParser {
  explicit TypeParserImpl(const std::string& filename);

  const std::vector<ObjectType>& getAllObjects() const override;
  Object getObject(ObjectTypeKey key) override;

 private:
  void genNames(Dwarf_Die die,
                Scope& scope,
                std::vector<Dwarf_Off>* template_params = nullptr);
  Object genObject(Dwarf_Die die,
                   ObjectTypeName name,
                   ObjectTypeKey key);
  Type genType(Dwarf_Die die);
  Object::Member genMember(Dwarf_Die die,
                           const ObjectTypeName& parent_name);
  Object::Function genFunction(Dwarf_Die die);
  Object::Base genBase(Dwarf_Die die, const ObjectTypeName& parent_name);
  Object::TemplateParam genTemplateParam(Dwarf_Die die);

  folly::Optional<std::size_t> determineArrayBound(Dwarf_Die die);
  folly::Optional<Dwarf_Off> findSpecification(Dwarf_Die die, bool first);

  void fillFuncArgs(Dwarf_Die die, FuncType& func);

  void fixTemplateLinkage();

  DwarfState m_dwarf;

  std::vector<ObjectType> m_all_objs;
  std::unordered_map<Dwarf_Off, std::size_t> m_offsets;

  struct LinkageDependents {
    std::unordered_set<Dwarf_Off> m_template_uses;
    std::unordered_set<Dwarf_Off> m_children;
  };
  std::unordered_map<Dwarf_Off, LinkageDependents> m_linkage_dependents;

  std::unordered_multimap<Dwarf_Off, Dwarf_Off> m_static_definitions;
};

// Purposefully fake name to avoid confusion with an actual type.
const std::string Scope::s_pseudo_type_name = "@_PSEUDO_TY";

DwarfState::DwarfState(const std::string& filename)
  : fd{-1}
  , dwarf{nullptr}
  , filename{filename}
{
  fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    throw Exception{
      folly::sformat(
        "Unable to open file '{}': {}",
        filename,
        folly::errnoStr(errno)
      )
    };
  }

  Dwarf_Error error = nullptr;
  if (dwarf_init(
        fd,
        DW_DLC_READ,
        nullptr,
        nullptr,
        &dwarf,
        &error
      ) == DW_DLV_ERROR) {
    SCOPE_EXIT { if (error) free(error); };
    SCOPE_EXIT { close(fd); };
    throw Exception{
      folly::sformat(
        "Unable to init libdwarf on file '{}': {}",
        filename,
        dwarf_errmsg(error)
      )
    };
  }
}

DwarfState::~DwarfState() {
  Dwarf_Error error = nullptr;
  if (dwarf) dwarf_finish(dwarf, &error);
  close(fd);
}

Dwarf_Half DwarfState::getTag(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half tag = 0;
  if (dwarf_tag(die, &tag, &error) != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to read DIE tag: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return tag;
}

std::string DwarfState::tagToString(Dwarf_Half tag) {
  const char* tag_name = nullptr;
  auto result = dwarf_get_TAG_name(tag, &tag_name);
  if (result == DW_DLV_NO_ENTRY) {
    return folly::sformat("<UNKNOWN({})>", tag);
  }
  return tag_name;
}

std::string DwarfState::getDIEName(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  char* raw_name = nullptr;
  auto result = dwarf_diename(die, &raw_name, &error);
  if (result == DW_DLV_ERROR) {
    throw Exception{
      folly::sformat(
          "Unable to read DIE name: {}",
          dwarf_errmsg(error)
      )
    };
  } else if (result == DW_DLV_NO_ENTRY) {
    return "";
  } else {
    SCOPE_EXIT { dwarf_dealloc(dwarf, raw_name, DW_DLA_STRING); };
    return raw_name;
  }
}

Dwarf_Off DwarfState::getDIEOffset(Dwarf_Die die) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Off offset = 0;
  auto result = dwarf_dieoffset(die, &offset, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
          "Unable to read DIE offset: {}",
          dwarf_errmsg(error)
      )
    };
  }

  return offset;
}

Dwarf_Half DwarfState::getAttributeType(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half what_attr = 0;
  auto result = dwarf_whatattr(attr, &what_attr, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute type: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return what_attr;
}

std::string DwarfState::attributeTypeToString(Dwarf_Half type) {
  const char* attr_name;
  auto result = dwarf_get_AT_name(type, &attr_name);
  if (result == DW_DLV_NO_ENTRY) {
    return folly::sformat("<UNKNOWN({})>", type);
  }
  return attr_name;
}

Dwarf_Half DwarfState::getAttributeForm(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Half form = 0;
  auto result = dwarf_whatform(attr, &form, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute form: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return form;
}

std::string DwarfState::getAttributeValueString(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  char* str = nullptr;
  auto result = dwarf_formstring(attr, &str, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value string: {}",
        dwarf_errmsg(error)
      )
    };
  }

  SCOPE_EXIT { dwarf_dealloc(dwarf, str, DW_DLA_STRING); };
  return str;
}

Dwarf_Bool DwarfState::getAttributeValueFlag(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  if (getAttributeForm(attr) == DW_FORM_flag_present) {
    return true;
  }

  Dwarf_Bool value;
  auto result = dwarf_formflag(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value flag: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Unsigned DwarfState::getAttributeValueUData(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Unsigned value;
  auto result = dwarf_formudata(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value unsigned data: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Signed DwarfState::getAttributeValueSData(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Signed value;
  auto result = dwarf_formsdata(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value signed data: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Addr DwarfState::getAttributeValueAddr(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Addr value;
  auto result = dwarf_formaddr(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value address: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Off DwarfState::getAttributeValueRef(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Off value;
  auto result = dwarf_global_formref(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value ref: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

Dwarf_Sig8 DwarfState::getAttributeValueSig8(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Sig8 value;
  auto result = dwarf_formsig8(attr, &value, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value sig8: {}",
        dwarf_errmsg(error)
      )
    };
  }

  return value;
}

std::vector<Dwarf_Loc>
DwarfState::getAttributeValueExprLoc(Dwarf_Attribute attr) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Ptr raw_ptr;
  Dwarf_Unsigned raw_len;
  auto result = dwarf_formexprloc(attr, &raw_len, &raw_ptr, &error);
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to obtain attribute value exprloc: {}",
        dwarf_errmsg(error)
      )
    };
  }

  Dwarf_Locdesc* locations = nullptr;
  Dwarf_Signed locations_count = 0;
  SCOPE_EXIT {
    if (locations) {
      for (Dwarf_Signed i = 0; i < locations_count; ++i) {
        dwarf_dealloc(dwarf, locations[i].ld_s, DW_DLA_LOC_BLOCK);
      }
      dwarf_dealloc(dwarf, locations, DW_DLA_LOCDESC);
    }
  };

  result = dwarf_loclist_from_expr(
    dwarf,
    raw_ptr,
    raw_len,
    &locations,
    &locations_count,
    &error
  );
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to turn exprloc into location list: {}",
        dwarf_errmsg(error)
      )
    };
  }

  if (locations_count != 1) {
    throw Exception{"Obtained more than one location list from exprloc"};
  }

  return std::vector<Dwarf_Loc>{
    locations->ld_s,
    locations->ld_s + locations->ld_cents
  };
}

/*
 * Iterate over all children of this DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F> void DwarfState::forEachChild(Dwarf_Die die, F&& f) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die prev = nullptr;
  SCOPE_EXIT {
    if (prev) dwarf_dealloc(dwarf, prev, DW_DLA_DIE);
  };

  if (die) {
    // prev is null here, and dwarf_child returns the first child if given a
    // previous DIE of null.
    auto result = dwarf_child(die, &prev, &error);
    if (result == DW_DLV_ERROR) {
      throw Exception{
        folly::sformat(
          "Unable to read child DIE: {}",
          dwarf_errmsg(error)
        )
      };
    } else if (result == DW_DLV_NO_ENTRY || !f(prev)) {
      return;
    }
  }

  while (true) {
    Dwarf_Die next = nullptr;
    SCOPE_EXIT {
      if (next) dwarf_dealloc(dwarf, next, DW_DLA_DIE);
    };

    auto result = dwarf_siblingof_b(
      dwarf, prev, true,
      &next, &error
    );
    if (result == DW_DLV_ERROR) {
      throw Exception{
        folly::sformat(
          "Unable to read sibling DIE: {}",
          dwarf_errmsg(error)
        )
      };
    } else if (result == DW_DLV_NO_ENTRY || !f(next)) {
      break;
    }

    // Swap prev and next. This will ensure the previous DIE gets freed (because
    // of the above SCOPE_EXIT).
    std::swap(prev, next);
  }
}

/*
 * Iterate over all attributes of the given DIE, calling the given callable for
 * each. Iteration is stopped early if any of the calls return false.
 */
template <typename F> void DwarfState::forEachAttribute(Dwarf_Die die, F&& f) {
  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Attribute* attributes;
  Dwarf_Signed attribute_count;
  auto result = dwarf_attrlist(die, &attributes, &attribute_count, &error);
  if (result == DW_DLV_ERROR) {
    throw Exception{
      folly::sformat(
        "Unable to read DIE attribute-list: {}",
        dwarf_errmsg(error)
      )
    };
  } else if (result == DW_DLV_NO_ENTRY) {
    return;
  }

  SCOPE_EXIT {
    for (Dwarf_Unsigned i = 0; i < attribute_count; ++i) {
      dwarf_dealloc(dwarf, attributes[i], DW_DLA_ATTR);
    }
    dwarf_dealloc(dwarf, attributes, DW_DLA_LIST);
  };

  for (Dwarf_Unsigned i = 0; i < attribute_count; ++i) {
    if (!f(attributes[i])) break;
  }
}

/*
 * Iterate over all the compilation-units in the file, calling the given
 * callable for each.
 */
template <typename F> void DwarfState::forEachCompilationUnit(F&& f) {
  if (!dwarf) return;

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  while (true) {
    Dwarf_Unsigned next_cu_header = 0;
    auto result = dwarf_next_cu_header_d(
      dwarf, true, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, &next_cu_header,
      nullptr, &error
    );

    if (result == DW_DLV_NO_ENTRY) {
      break;
    } else if (result == DW_DLV_ERROR) {
      throw Exception{
        folly::sformat(
          "Unable to read next compilation-unit header: {}",
          dwarf_errmsg(error)
        )
      };
    }

    forEachChild(
      nullptr,
      [&](Dwarf_Die die){
        if (getTag(die) != DW_TAG_compile_unit) {
          throw Exception{
            folly::sformat(
              "First tag in compilation-unit is not DW_TAG_compile_unit ({})",
              tagToString(getTag(die))
            )
          };
        }
        f(die);
        return true;
      }
    );
  }
}

/*
 * Load the DIE at the given offset, and call the given callable on it,
 * returning whatever the callable returns.
 */
template <typename F> auto DwarfState::onDIEAtOffset(Dwarf_Off offset, F&& f) ->
  decltype(f(std::declval<Dwarf_Die>())) {

  Dwarf_Error error = nullptr;
  SCOPE_EXIT {
    if (error) dwarf_dealloc(dwarf, error, DW_DLA_ERROR);
  };

  Dwarf_Die die = nullptr;
  auto result = dwarf_offdie_b(
    dwarf, offset, true,
    &die, &error
  );
  if (result != DW_DLV_OK) {
    throw Exception{
      folly::sformat(
        "Unable to read DIE at offset {}: {}",
        offset,
        dwarf_errmsg(error)
      )
    };
  }

  SCOPE_EXIT { dwarf_dealloc(dwarf, die, DW_DLA_DIE); };
  return f(die);
}

ObjectTypeName Scope::name() const {
  auto iter = m_scope.begin();
  std::string str = iter->name.name;
  ++iter;
  for (; iter != m_scope.end(); ++iter) {
    if (str.empty()) str = iter->name.name;
    else str = folly::sformat("{}::{}", str, iter->name.name);
  }
  return ObjectTypeName{std::move(str), linkage()};
}

TypeParserImpl::TypeParserImpl(const std::string& filename)
  : m_dwarf{filename}
{
  // First iterate over every DIE, finding all object types and computing their
  // names (and linkage).
  m_dwarf.forEachCompilationUnit(
    [&](Dwarf_Die cu) {
      Scope scope{m_dwarf.getDIEOffset(cu)};
      genNames(cu, scope);
    }
  );

  // Compute a mapping of an object type's offset to its location in the
  // m_all_objs vector.
  for (std::size_t i = 0; i < m_all_objs.size(); ++i) {
    m_offsets.emplace(m_all_objs[i].key.object_id, i);
  }

  // The linkage of template instantiations need to be fixed up depending on the
  // linkage of its template parameters. Now that we have all the types, do so
  // here.
  fixTemplateLinkage();
  m_linkage_dependents.clear();
}

const std::vector<ObjectType>& TypeParserImpl::getAllObjects() const {
  return m_all_objs;
}

/*
 * As stated above, the linkage of templates is tricky. The linkage of a
 * template is the most restrictive linkage of its original linkage and the
 * linkage of its template parameters. Since some of the template parameters may
 * not yet be parsed when we parse the template, the inference of the correct
 * template linkage is deferred until all the types' linkages are computed.
 *
 * However, since templates can be parameters to other templates, this process
 * must be repeated until the linkage of no types are changed.
 *
 * As an additional complication, the linkage of any nested class is inherited
 * from its parent, so when a template's linkage changes, it must be bubbled
 * down to any of its nested classes.
 *
 * When the name and initial linkages of all the types was generated, the
 * relationship between templates, their parameters, and nested classes is
 * recorded in m_linkage_dependents, which is used here.
 */
void TypeParserImpl::fixTemplateLinkage() {
  std::unordered_set<Dwarf_Off> changed;

  for (const auto& pair : m_linkage_dependents) {
    if (pair.second.m_template_uses.empty()) continue;
    changed.emplace(pair.first);
  }

  std::unordered_set<Dwarf_Off> old_changed;
  while (!changed.empty()) {
    std::swap(changed, old_changed);

    // For every type which has its linkage changed, update its dependents
    // (templates where the type is used as a parameter, or nested classes) with
    // the new linkage, and mark as being changed as well.
    for (auto changed_offset : old_changed) {
      const auto iter = m_linkage_dependents.find(changed_offset);
      if (iter == m_linkage_dependents.end()) continue;

      const auto& children = iter->second.m_children;
      const auto& template_uses = iter->second.m_template_uses;

      const auto& changed_obj = m_all_objs[m_offsets[changed_offset]];

      // Only update and mark if we actually make the linkage more restrictive.
      switch (changed_obj.name.linkage) {
        case ObjectTypeName::Linkage::external:
          break;
        case ObjectTypeName::Linkage::internal: {
          const auto process = [&](Dwarf_Off dependent_offset) {
            auto& dependent_obj = m_all_objs[m_offsets[dependent_offset]];
            switch (dependent_obj.name.linkage) {
              case ObjectTypeName::Linkage::external:
                dependent_obj.name.linkage = changed_obj.name.linkage;
                changed.emplace(dependent_offset);
                break;
              case ObjectTypeName::Linkage::internal:
              case ObjectTypeName::Linkage::none:
              case ObjectTypeName::Linkage::pseudo:
                break;
            }
          };
          for (auto template_offset : template_uses) process(template_offset);
          for (auto child_offset : children) process(child_offset);
        }
        case ObjectTypeName::Linkage::none: {
          const auto process = [&](Dwarf_Off dependent_offset) {
            auto& dependent_obj = m_all_objs[m_offsets[dependent_offset]];
            switch (dependent_obj.name.linkage) {
              case ObjectTypeName::Linkage::external:
              case ObjectTypeName::Linkage::internal:
                dependent_obj.name.linkage = changed_obj.name.linkage;
                changed.emplace(dependent_offset);
                break;
              case ObjectTypeName::Linkage::none:
              case ObjectTypeName::Linkage::pseudo:
                break;
            }
          };
          for (auto template_offset : template_uses) process(template_offset);
          for (auto child_offset : children) process(child_offset);
        }
        case ObjectTypeName::Linkage::pseudo: {
          const auto process = [&](Dwarf_Off dependent_offset) {
            auto& dependent_obj = m_all_objs[m_offsets[dependent_offset]];
            switch (dependent_obj.name.linkage) {
              case ObjectTypeName::Linkage::external:
              case ObjectTypeName::Linkage::internal:
              case ObjectTypeName::Linkage::none:
                dependent_obj.name.linkage = changed_obj.name.linkage;
                changed.emplace(dependent_offset);
                break;
              case ObjectTypeName::Linkage::pseudo:
                break;
            }
          };
          for (auto template_offset : template_uses) process(template_offset);
          for (auto child_offset : children) process(child_offset);
        }
      }
    }

    old_changed.clear();
  }
}

Object TypeParserImpl::getObject(ObjectTypeKey key) {
  auto iter = m_offsets.find(key.object_id);
  // If we don't know of an object type at the given location, assume its
  // referring to something we never parsed in the first place, so return the
  // pseudo-type.
  if (iter == m_offsets.end()) {
    return Object{
      ObjectTypeName{
        Scope::s_pseudo_type_name,
        ObjectTypeName::Linkage::pseudo,
      },
      0,
      key,
      Object::Kind::k_other,
      true
     };
  }

  return m_dwarf.onDIEAtOffset(
    key.object_id,
    [&](Dwarf_Die die) {
      return genObject(
        die,
        m_all_objs[iter->second].name,
        key
      );
    }
  );
}

/*
 * Static definitions have a "specification" which basically links the
 * definition to the declaration. A specification can refer to another
 * specification, so they form a chain. Given a DIE, walk the specification
 * chain and return the offset of the actual type declaration.
 */
folly::Optional<Dwarf_Off> TypeParserImpl::findSpecification(Dwarf_Die die,
                                                             bool first) {
  folly::Optional<Dwarf_Off> offset;
  bool is_inline = false;
  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_abstract_origin:
          offset = m_dwarf.onDIEAtOffset(
            m_dwarf.getAttributeValueRef(attr),
            [&](Dwarf_Die die) { return findSpecification(die, false); }
          );
          break;
        case DW_AT_specification:
          offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_inline:
          is_inline = true;
          break;
        default:
          break;
      }
      return true;
    }
  );
  if (first && is_inline) return folly::none;
  return offset;
}

/*
 * Given a DIE, and the current scope, recursively generate the names/linkages
 * for all the object types in this DIE and children. If template_params is
 * provided, the parent DIE is an object type, so template_params should be
 * filled with any template parameters in the child DIE.
 */
void TypeParserImpl::genNames(Dwarf_Die die,
                              Scope& scope,
                              std::vector<Dwarf_Off>* template_params) {
  const auto recurse = [&](std::vector<Dwarf_Off>* params = nullptr){
    m_dwarf.forEachChild(
      die,
      [&](Dwarf_Die child) { genNames(child, scope, params); return true; }
    );
  };

  auto tag = m_dwarf.getTag(die);
  switch (tag) {
    case DW_TAG_base_type:
    case DW_TAG_union_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_structure_type:
    case DW_TAG_class_type:
    case DW_TAG_unspecified_type: {
      // Object-types. These have names and linkages, so we must record them.

      // Determine the base name and whether this is an incomplete type or not
      // from the DIE's attributes.
      const auto info = [&]() -> std::pair<std::string, bool> {
        std::string name;
        std::string linkage_name;
        bool incomplete = false;

        m_dwarf.forEachAttribute(
          die,
          [&](Dwarf_Attribute attr) {
            switch (m_dwarf.getAttributeType(attr)) {
              case DW_AT_name:
                name = m_dwarf.getAttributeValueString(attr);
                break;
              case DW_AT_linkage_name:
                linkage_name = m_dwarf.getAttributeValueString(attr);
                break;
              case DW_AT_declaration:
                incomplete = m_dwarf.getAttributeValueFlag(attr);
                break;
              default:
                break;
            }
            return true;
          }
        );

        // If there's an explicit name, just use that.
        if (!name.empty()) return {name, incomplete};

        // Otherwise, if there's a linkage name, demangle it, and strip off
        // everything except the last section, and use that as the base
        // name. For types which have external linkage, this lets us use
        // whatever naming scheme the compiler has chosen for unnamed types.
        if (!linkage_name.empty()) {
          auto demangled = folly::demangle(linkage_name.c_str()).toStdString();
          auto index = demangled.rfind("::");
          if (index != decltype(demangled)::npos) demangled.erase(0, index+2);
          return {demangled, incomplete};
        }

        // No explicit name and no linkage name to use, so we have to try to
        // infer one ourself.

        // If this is an enumeration, use the first enumerator.
        if (tag == DW_TAG_enumeration_type) {
          std::string enumerator;
          m_dwarf.forEachChild(
            die,
            [&](Dwarf_Die child) {
              if (m_dwarf.getTag(child) == DW_TAG_enumerator) {
                enumerator = m_dwarf.getDIEName(child);
              }
              return enumerator.empty();
            }
          );
          if (!enumerator.empty()) return {enumerator, incomplete};
        }

        // If this is an union, use the first union member.
        if (tag == DW_TAG_union_type) {
          std::string first_member;
          m_dwarf.forEachChild(
            die,
            [&](Dwarf_Die child) {
              if (m_dwarf.getTag(child) == DW_TAG_member) {
                first_member = m_dwarf.getDIEName(child);
              }
              return first_member.empty();
            }
          );
          if (!first_member.empty()) return {first_member, incomplete};
        }

        // If this is within a namespace, don't infer any name at all, keep it
        // nameless. If its not within a namespace (IE, within a class), give it
        // a unique name based on how many unnamed types we've seen so far. We
        // can't do this for types within a namespace because namespaces are
        // open and thus we can't force a global numbering of all types within
        // it.
        if (!scope.isInNamespaceScope()) {
          scope.incUnnamedTypeCount();
          return {
            folly::sformat("(anonymous type #{})", scope.unnamedTypeCount()),
            incomplete
          };
        } else return {std::string{}, incomplete};
      }();

      auto offset = m_dwarf.getDIEOffset(die);
      auto parent_offset = scope.typeOffset();

      // If we inferred a base name, use that to form the fully qualified name,
      // otherwise treat it as an anonymous type.
      info.first.empty() ?
        scope.pushAnonType(offset) :
        scope.pushType(info.first, offset);
      SCOPE_EXIT { scope.pop(); };

      // Record this object type, with fully qualified name, key, and linkage.
      m_all_objs.emplace_back(
        ObjectType{
          scope.name(),
          ObjectTypeKey{offset, scope.cuOffset()},
          info.second
        }
      );

      // This object type is done, so recurse into any nested classes. Provide a
      // list of template parameters to be filled in case this is a template. If
      // it is, we'll record the linkage dependence for the later template
      // linkage fix-up.
      std::vector<Dwarf_Off> recurse_template_params;
      recurse(&recurse_template_params);

      for (auto param_offset : recurse_template_params) {
        m_linkage_dependents[param_offset].m_template_uses.emplace(offset);
      }
      if (parent_offset) {
        m_linkage_dependents[*parent_offset].m_children.emplace(offset);
      }
      break;
    }
    case DW_TAG_namespace: {
      // Record the namespace in the scope and recurse. If this is an anonymous
      // namespace, that means any type found in child DIEs will have internal
      // linkage.
      auto name = m_dwarf.getDIEName(die);
      name.empty() ?
        scope.pushAnonNamespace() :
        scope.pushNamespace(std::move(name));
      SCOPE_EXIT { scope.pop(); };
      recurse();
      break;
    }
    case DW_TAG_variable: {
      // Normally we don't care about variables since we're only looking for
      // types. However, certain aspects of object types can't be completely
      // inferred at the declaration site (mainly static variable linkage
      // related things like linkage name and address). We need a definition for
      // that, so record all the variable definitions along with their
      // specification, which we can consult later.

      // Neither GCC nor Clang record a name for a variable which is a static
      // definition, so ignore any that do have a name. This speeds things up.
      if (!m_dwarf.getDIEName(die).empty()) break;

      m_dwarf.forEachAttribute(
        die,
        [&](Dwarf_Attribute attr) {
          switch (m_dwarf.getAttributeType(attr)) {
            case DW_AT_specification:
              m_static_definitions.emplace(
                m_dwarf.getAttributeValueRef(attr),
                m_dwarf.getDIEOffset(die)
              );
              return false;
            default:
              return true;
          }
        }
      );

      // Note that we don't recurse into any child DIEs here. There shouldn't be
      // anything interesting in them.
      break;
    }
    case DW_TAG_subprogram: {
      // For the same reason we care about DW_TAG_variables, we examine
      // DW_TAG_subprogram as well. Certain interesting aspects of a static
      // function are only present in its definition.

      if (!m_dwarf.getDIEName(die).empty()) break;

      if (auto spec = findSpecification(die, true)) {
        m_static_definitions.emplace(
          *spec,
          m_dwarf.getDIEOffset(die)
        );
      }

      // Don't recurse. There might be valid types within a subprogram
      // definition, but we deliberate ignore those. A large portion of the
      // debug information lies within subprogram definitions, and scanning all
      // of that consumes a large amount of time. Moreover, these types usually
      // aren't very interesting, so we deliberately ignore them for
      // efficiency. If there's actually any reference to these types, they'll
      // be reported as the pseudo-type.
      break;
    }
    case DW_TAG_template_type_param: {
      // Template type parameters are represented using child DIEs, not
      // attributes. If the parent DIE was an object type, fill the supplied
      // vector with the template parameters. Don't recurse because there
      // shouldn't be anything interesting in the children.
      if (template_params) {
        m_dwarf.forEachAttribute(
          die,
          [&](Dwarf_Attribute attr) {
            switch (m_dwarf.getAttributeType(attr)) {
              case DW_AT_type:
                template_params->emplace_back(
                  m_dwarf.getAttributeValueRef(attr)
                );
                return false;
              default:
                return true;
            }
          }
        );
      }
      break;
    }
    default:
      recurse();
      break;
  }
}

/*
 * Given the DIE representing an object type, its name, and its key, return the
 * detailed specification of the object.
 */
Object TypeParserImpl::genObject(Dwarf_Die die,
                                 ObjectTypeName name,
                                 ObjectTypeKey key) {
  const auto kind = [&]{
    switch (m_dwarf.getTag(die)) {
      case DW_TAG_structure_type: return Object::Kind::k_class;
      case DW_TAG_class_type: return Object::Kind::k_class;
      case DW_TAG_union_type: return Object::Kind::k_union;
      case DW_TAG_base_type: return Object::Kind::k_primitive;
      case DW_TAG_enumeration_type: return Object::Kind::k_enum;
      // Strange things like "decltype(nullptr_t)"
      case DW_TAG_unspecified_type: return Object::Kind::k_other;
      // Shouldn't happen because we only call genObject() on offsets already
      // visited and verified to be an object type.
      default: always_assert(0);
    }
  }();

  folly::Optional<std::size_t> size;
  bool incomplete = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_byte_size:
          size = m_dwarf.getAttributeValueUData(attr);
          break;
        case DW_AT_declaration:
          incomplete = m_dwarf.getAttributeValueFlag(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  // No size was provided. This is expected for incomplete types or the strange
  // "other" types sometimes seen, but an error otherwise.
  if (!size) {
    if (incomplete || kind == Object::Kind::k_other) {
      size = 0;
    } else {
      throw Exception{
        folly::sformat(
          "Object type '{}' at offset {} is a complete definition, "
          "but has no size!",
          name.name,
          key.object_id
        )
      };
    }
  }

  Object obj{std::move(name), *size, key, kind, incomplete};

  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_inheritance:
          obj.bases.emplace_back(genBase(child, obj.name));
          break;
        case DW_TAG_member:
          obj.members.emplace_back(genMember(child, obj.name));
          if (obj.name.linkage != ObjectTypeName::Linkage::external) {
            // Clang gives linkage names to things that don't actually have
            // linkage. Don't let any members have linkage names if the object
            // type doesn't have external linkage.
            obj.members.back().linkage_name.clear();
          }
          break;
        case DW_TAG_template_type_parameter:
          obj.template_params.emplace_back(genTemplateParam(child));
          break;
        case DW_TAG_GNU_template_parameter_pack:
          // Flatten parameter packs as if they were just a normally provided
          // parameter list. This is enough for our purposes.
          m_dwarf.forEachChild(
            child,
            [&](Dwarf_Die template_die) {
              if (m_dwarf.getTag(template_die) ==
                  DW_TAG_template_type_parameter) {
                obj.template_params.emplace_back(
                  genTemplateParam(template_die)
                );
              }
              return true;
            }
          );
          break;
        case DW_TAG_subprogram:
          obj.functions.emplace_back(genFunction(child));
          if (obj.name.linkage != ObjectTypeName::Linkage::external) {
            // Clang gives linkage names to things that don't actually have
            // linkage. Don't let any functions have linkage names if the object
            // type doesn't have external linkage.
            obj.functions.back().linkage_name.clear();
          }
          break;
        default:
          break;
      }
      return true;
    }
  );

  // The base classes and members aren't always reported in DWARF in offset
  // order, but make the output deterministic here to simplify consumers of the
  // information.
  std::sort(
    obj.bases.begin(),
    obj.bases.end(),
    [&](const Object::Base& b1, const Object::Base& b2) {
      return std::tie(b1.offset, b1.type.name.name) <
        std::tie(b2.offset, b2.type.name.name);
    }
  );

  std::sort(
    obj.members.begin(),
    obj.members.end(),
    [&](const Object::Member& m1, const Object::Member& m2) {
      return std::tie(m1.offset, m1.name) <
        std::tie(m2.offset, m2.name);
    }
  );

  return obj;
}

/*
 * Given a DIE representing an arbitrary type, return its equivalent Type. This
 * can involve chasing a chain of such type DIEs.
 */
Type TypeParserImpl::genType(Dwarf_Die die) {
  // Offset of a different type this type refers to. If not present, that type
  // is implicitly "void".
  folly::Optional<Dwarf_Off> type_offset;
  // For pointers to members, the type referring to the object the member
  // belongs to.
  folly::Optional<Dwarf_Off> containing_type_offset;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_type:
          type_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_containing_type:
          containing_type_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  const auto recurse = [&](Dwarf_Off offset) {
    return m_dwarf.onDIEAtOffset(
      offset,
      [&](Dwarf_Die die) { return genType(die); }
    );
  };

  // Pointers to member functions aren't represented in DWARF. Instead the
  // compiler creates a struct internally which stores all the information.

  switch (m_dwarf.getTag(die)) {
    case DW_TAG_base_type:
    case DW_TAG_structure_type:
    case DW_TAG_class_type:
    case DW_TAG_union_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_unspecified_type: {
      auto offset = m_dwarf.getDIEOffset(die);
      auto iter = m_offsets.find(offset);
      if (iter == m_offsets.end()) {
        // Must be the pseudo-type.
        return ObjectType{
          ObjectTypeName{
            Scope::s_pseudo_type_name,
            ObjectTypeName::Linkage::pseudo
          },
          ObjectTypeKey{offset, 0},
          true
        };
      } else {
        return m_all_objs[iter->second];
      }
    }
    case DW_TAG_pointer_type:
      return PtrType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_reference_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered reference to void at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return RefType{recurse(*type_offset)};
    }
    case DW_TAG_rvalue_reference_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered rvalue reference to void at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return RValueRefType{recurse(*type_offset)};
    }
    case DW_TAG_array_type: {
      if (!type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered array of voids at offset {}",
            m_dwarf.getDIEOffset(die)
          )
        };
      }
      return ArrType{recurse(*type_offset), determineArrayBound(die)};
    }
    case DW_TAG_const_type:
      return ConstType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_volatile_type:
      return VolatileType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_restrict_type:
      return RestrictType{type_offset ? recurse(*type_offset) : VoidType{}};
    case DW_TAG_typedef:
      return type_offset ? recurse(*type_offset) : VoidType{};
    case DW_TAG_subroutine_type: {
      FuncType func{type_offset ? recurse(*type_offset) : VoidType{}};
      fillFuncArgs(die, func);
      return std::move(func);
    }
    case DW_TAG_ptr_to_member_type: {
      if (!containing_type_offset) {
        throw Exception{
          folly::sformat(
            "Encountered ptr-to-member at offset {} without a "
            "containing object",
            m_dwarf.getDIEOffset(die)
          )
        };
      }

      auto containing = recurse(*containing_type_offset);
      if (auto obj = containing.asObject()) {
        return PtrType{
          MemberType{std::move(*obj), recurse(*type_offset)}
        };
      } else {
        throw Exception{
          folly::sformat(
            "Encountered ptr-to-member at offset {} with a "
            "containing object of type '{}'",
            m_dwarf.getDIEOffset(die),
            containing.toString()
          )
        };
      }
    }
    default:
      throw Exception{
        folly::sformat(
          "Encountered non-type tag '{}' at offset {} while "
          "traversing type description",
          m_dwarf.tagToString(m_dwarf.getTag(die)),
          m_dwarf.getDIEOffset(die)
        )
      };
  }
}

Object::Member TypeParserImpl::genMember(Dwarf_Die die,
                                         const ObjectTypeName& parent_name) {
  std::string name;
  std::string linkage_name;
  std::size_t offset = 0;
  folly::Optional<Dwarf_Off> die_offset;
  folly::Optional<uintptr_t> address;
  bool is_static = false;

  // For static members, determine how that member's address can be
  // determined. In theory, this can be any arbitrary expression, but we only
  // support constant addresses right now.
  const auto interpret_loc_address = [&](Dwarf_Attribute attr)
    -> folly::Optional<uintptr_t> {
    auto form = m_dwarf.getAttributeForm(attr);
    if (form != DW_FORM_exprloc) return folly::none;
    auto exprs = m_dwarf.getAttributeValueExprLoc(attr);
    if (exprs.size() != 1) return folly::none;
    if (exprs[0].lr_atom != DW_OP_addr) return folly::none;
    return folly::Optional<uintptr_t>{exprs[0].lr_number};
  };

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_linkage_name:
          linkage_name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_location:
          address = interpret_loc_address(attr);
          break;
        case DW_AT_data_member_location:
          offset = m_dwarf.getAttributeValueUData(attr);
          break;
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_declaration:
          is_static = m_dwarf.getAttributeValueFlag(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (!die_offset) {
    // No DW_AT_type means "void", but you can't have void members!
    throw Exception{
      folly::sformat(
        "Encountered member (name: '{}') of type void "
        "in object type '{}' at offset {}",
        name,
        parent_name.name,
        m_dwarf.getDIEOffset(die)
      )
    };
  }

  if (is_static) {
    // If this is a static member, look up any definitions which refer to this
    // member, and pull any additional information out of it.
    auto range = m_static_definitions.equal_range(m_dwarf.getDIEOffset(die));
    auto count = std::distance(range.first, range.second);

    if (count > 1) {
      // Multiple definitions? Technically okay if their information isn't
      // contradictory, but doesn't seem to happen.
      throw Exception{
        folly::sformat(
          "Encountered static member (name: '{}') with "
          "multiple definitions in object type '{}' at offset{}",
          name,
          parent_name.name,
          m_dwarf.getDIEOffset(die)
        )
      };
    }

    // Only extract linkage information from the definition.
    if (count > 0) {
      m_dwarf.onDIEAtOffset(
        range.first->second,
        [&](Dwarf_Die die) {
          m_dwarf.forEachAttribute(
            die,
            [&](Dwarf_Attribute attr) {
              switch (m_dwarf.getAttributeType(attr)) {
                case DW_AT_linkage_name:
                  if (linkage_name.empty()) {
                    linkage_name = m_dwarf.getAttributeValueString(attr);
                  }
                  break;
                case DW_AT_location:
                  if (!address) address = interpret_loc_address(attr);
                  break;
                default:
                  break;
              }
              return true;
            }
          );
        }
      );
    }

  }

  return Object::Member{
    name,
    is_static ? folly::none : folly::Optional<std::size_t>{offset},
    linkage_name,
    address,
    m_dwarf.onDIEAtOffset(
      *die_offset,
      [&](Dwarf_Die die){ return genType(die); }
    )
  };
}

Object::Function TypeParserImpl::genFunction(Dwarf_Die die) {
  std::string name;
  Type ret_type{VoidType{}};
  std::string linkage_name;
  bool is_virtual = false;
  bool is_member = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_type:
          ret_type = m_dwarf.onDIEAtOffset(
            m_dwarf.getAttributeValueRef(attr),
            [&](Dwarf_Die ty_die) { return genType(ty_die); }
          );
          break;
        case DW_AT_linkage_name:
          linkage_name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_virtuality:
          is_virtual =
            (m_dwarf.getAttributeValueUData(attr) != DW_VIRTUALITY_none);
          break;
        case DW_AT_object_pointer:
          is_member = true;
          break;
        default:
          break;
      }
      return true;
    }
  );

  /*
   * We need to determine if this function is a static function or a member
   * function. The straight-forward way is to look for the DW_AT_object_pointer
   * attribute (which is only present for member functions). This works fine for
   * GCC, but not Clang.
   *
   * On Clang, the DW_AT_object_pointer is only present in a function's
   * definition, not its declaration. Moreover, it doesn't reliably emit
   * function declarations if it thinks the function isn't used. As a result, we
   * can't reliably distinguish member functions for static functions on clang.
   *
   * As an alternative, if the first formal parameter of a function is marked as
   * being "artificial" (which means its not present in the actual source),
   * assume its actually the this pointer, and that the function is a member
   * function.
   */
  std::vector<Type> arg_types;
  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      if (m_dwarf.getTag(child) != DW_TAG_formal_parameter) {
        return true;
      }

      bool is_artificial = false;
      Type arg_type{VoidType()};

      m_dwarf.forEachAttribute(
        child,
        [&](Dwarf_Attribute attr) {
          switch (m_dwarf.getAttributeType(attr)) {
            case DW_AT_type:
              arg_type = m_dwarf.onDIEAtOffset(
                  m_dwarf.getAttributeValueRef(attr),
                  [&](Dwarf_Die ty_die) { return genType(ty_die); }
              );
              break;
            case DW_AT_artificial:
              is_artificial = m_dwarf.getAttributeValueFlag(attr);
              break;
            default:
              break;
          }
          return true;
        }
      );

      // Only consider this a member function if this arg if the first and its
      // artificial.
      if (is_artificial && arg_types.empty()) {
        is_member = true;
      }
      arg_types.emplace_back(std::move(arg_type));

      return true;
    }
  );

  folly::Optional<std::uintptr_t> address;

  // Similar to static variables, find any definitions which refer to this
  // function in order to extract linkage information. Unlike static variables,
  // there can be multiple definitions, but we'll only take the first
  // information we see.
  auto range = m_static_definitions.equal_range(m_dwarf.getDIEOffset(die));
  while (range.first != range.second) {
    m_dwarf.onDIEAtOffset(
      range.first->second,
      [&](Dwarf_Die die) {
        m_dwarf.forEachAttribute(
          die,
          [&](Dwarf_Attribute attr) {
            switch (m_dwarf.getAttributeType(attr)) {
              case DW_AT_linkage_name:
                if (linkage_name.empty()) {
                  linkage_name = m_dwarf.getAttributeValueString(attr);
                }
                break;
              case DW_AT_low_pc:
                if (!address) {
                  address = m_dwarf.getAttributeValueAddr(attr);
                  // Sometimes GCC and Clang will emit invalid function
                  // addresses. Usually zero, but sometimes a very low
                  // number. These numbers have the appearance of being
                  // un-relocated addresses, but its in the final executable. As
                  // a safety net, if an address is provided, but its abnormally
                  // low, ignore it.
                  if (*address < 4096) address.clear();
                }
                break;
              case DW_AT_object_pointer:
                // Just in case we actually have a definition, use it to infer
                // member-ness.
                is_member = true;
                break;
              default:
                break;
            }
            return true;
          }
        );
      }
    );

    ++range.first;
  }

  return Object::Function{
    name,
    std::move(ret_type),
    std::move(arg_types),
    is_virtual ?
      Object::Function::Kind::k_virtual :
      (is_member ? Object::Function::Kind::k_member :
       Object::Function::Kind::k_static),
    linkage_name,
    address,
  };
}

Object::Base TypeParserImpl::genBase(Dwarf_Die die,
                                     const ObjectTypeName& parent_name) {
  std::string name;
  folly::Optional<std::size_t> offset;
  folly::Optional<Dwarf_Off> die_offset;
  bool is_virtual = false;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_name:
          name = m_dwarf.getAttributeValueString(attr);
          break;
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        case DW_AT_virtuality:
          is_virtual =
            (m_dwarf.getAttributeValueUData(attr) != DW_VIRTUALITY_none);
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (!is_virtual) {
    offset = 0;

    m_dwarf.forEachAttribute(
      die,
      [&](Dwarf_Attribute attr) {
        switch (m_dwarf.getAttributeType(attr)) {
          case DW_AT_data_member_location:
            offset = m_dwarf.getAttributeValueUData(attr);
            break;
          default:
            break;
        }
        return true;
      }
    );
  }

  if (!die_offset) {
    throw Exception{
      folly::sformat(
        "Encountered base '{}' of object type '{}' without "
        "type information at offset {}",
        name,
        parent_name.name,
        m_dwarf.getDIEOffset(die)
      )
    };
  }

  auto type =
    m_dwarf.onDIEAtOffset(
      *die_offset,
      [&](Dwarf_Die die) { return genType(die); }
    );

  if (auto obj = type.asObject()) {
    // Base class better be an actual class!
    return Object::Base{*obj, offset};
  } else {
    throw Exception{
      folly::sformat(
        "Encountered base '{}' of object type '{}' of "
        "non-object type '{}' at offset {}",
        name,
        parent_name.name,
        type.toString(),
        m_dwarf.getDIEOffset(die)
      )
    };
  }
}

Object::TemplateParam TypeParserImpl::genTemplateParam(Dwarf_Die die) {
  folly::Optional<Dwarf_Off> die_offset;

  m_dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      switch (m_dwarf.getAttributeType(attr)) {
        case DW_AT_type:
          die_offset = m_dwarf.getAttributeValueRef(attr);
          break;
        default:
          break;
      }
      return true;
    }
  );

  return Object::TemplateParam{
    die_offset ?
      m_dwarf.onDIEAtOffset(
        *die_offset,
        [&](Dwarf_Die die){ return genType(die); }
      ) :
      VoidType{}
  };
}

folly::Optional<std::size_t>
TypeParserImpl::determineArrayBound(Dwarf_Die die) {
  folly::Optional<std::size_t> bound;

  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_subrange_type:
          m_dwarf.forEachAttribute(
            child,
            [&](Dwarf_Attribute attr) {
              switch (m_dwarf.getAttributeType(attr)) {
                case DW_AT_count:
                  bound = m_dwarf.getAttributeValueUData(attr);
                  break;
                case DW_AT_upper_bound:
                  bound = m_dwarf.getAttributeValueUData(attr)+1;
                  break;
                default:
                  break;
              }
              return true;
            }
          );
          break;
        default:
          break;
      }
      return true;
    }
  );

  if (bound && !*bound) bound.clear();
  return bound;
}

void TypeParserImpl::fillFuncArgs(Dwarf_Die die, FuncType& func) {
  m_dwarf.forEachChild(
    die,
    [&](Dwarf_Die child) {
      switch (m_dwarf.getTag(child)) {
        case DW_TAG_formal_parameter: {
          folly::Optional<Dwarf_Off> type_offset;

          m_dwarf.forEachAttribute(
            child,
            [&](Dwarf_Attribute attr) {
              switch (m_dwarf.getAttributeType(attr)) {
                case DW_AT_type:
                  type_offset = m_dwarf.getAttributeValueRef(attr);
                  break;
                default:
                  break;
              }
              return true;
            }
          );

          if (!type_offset) {
            throw Exception{
              folly::sformat(
                "Encountered function at offset {} taking a void parameter",
                m_dwarf.getDIEOffset(die)
              )
            };
          }

          func.args.push_back(
            m_dwarf.onDIEAtOffset(
              *type_offset,
              [&](Dwarf_Die die) { return genType(die); }
            )
          );
          break;
        }
        default:
          break;
      }
      return true;
    }
  );
}

/*
 * Print out the given DIE (including children) in textual format to the given
 * ostream. Only actually print out DIEs which begin in the range between the
 * begin and end parameters.
 */

void printDIE(std::ostream& os,
              DwarfState& dwarf,
              Dwarf_Die die,
              std::size_t begin,
              std::size_t end,
              int indent = 0) {
  auto tag = dwarf.getTag(die);
  auto tag_name = dwarf.tagToString(tag);
  auto name = dwarf.getDIEName(die);
  auto offset = dwarf.getDIEOffset(die);

  const auto recurse = [&]{
    // Find the last child DIE which does not start with the begin/end
    // range. This DIE is the first one which contains some data within the
    // begin/end range, so that must be the first one to begin recursion at.
    folly::Optional<Dwarf_Off> first;
    if (begin > 0) {
      dwarf.forEachChild(
        die,
        [&](Dwarf_Die child) {
          const auto offset = dwarf.getDIEOffset(child);
          if (offset <= begin) {
            first = offset;
            return true;
          } else {
            return false;
          }
        }
      );
    }

    // Only actually recurse if this child DIE is the above computed first DIE,
    // or one following it, and begins before the end parameter.
    dwarf.forEachChild(
      die,
      [&](Dwarf_Die child) {
        const auto offset = dwarf.getDIEOffset(child);
        if ((!first || offset >= *first) && offset < end) {
          printDIE(os, dwarf, child, begin, end, indent+1);
        }
        return offset < end;
      }
    );
  };

  if (offset < begin) {
    recurse();
    return;
  } else if (offset >= end) {
    return;
  }

  for (int i = 0; i < indent; ++i) {
    os << "  ";
  }
  os << "#" << offset << ": " << tag_name << " (" << tag << ") \""
     << name << "\"\n";

  dwarf.forEachAttribute(
    die,
    [&](Dwarf_Attribute attr) {
      auto type = dwarf.getAttributeType(attr);
      auto attr_name = dwarf.attributeTypeToString(type);

      auto attr_value = [&]() -> std::string {
        switch (dwarf.getAttributeForm(attr)) {
          case DW_FORM_data1:
          case DW_FORM_data2:
          case DW_FORM_data4:
          case DW_FORM_data8:
          case DW_FORM_udata:
            return folly::sformat("{}", dwarf.getAttributeValueUData(attr));

          case DW_FORM_sdata:
            return folly::sformat("{}", dwarf.getAttributeValueSData(attr));

          case DW_FORM_string:
          case DW_FORM_strp:
            return folly::sformat(
              "\"{}\"",
              dwarf.getAttributeValueString(attr)
            );

          case DW_FORM_flag:
          case DW_FORM_flag_present:
            return dwarf.getAttributeValueFlag(attr) ? "true" : "false";

          case DW_FORM_addr:
            return folly::sformat(
              "{:#010x}",
              dwarf.getAttributeValueAddr(attr)
            );

          case DW_FORM_ref1:
          case DW_FORM_ref2:
          case DW_FORM_ref4:
          case DW_FORM_ref8:
          case DW_FORM_ref_udata:
          case DW_FORM_ref_addr:
            return folly::sformat("#{}", dwarf.getAttributeValueRef(attr));
          case DW_FORM_ref_sig8: {
            auto sig = dwarf.getAttributeValueSig8(attr);
            return folly::sformat(
              "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
              static_cast<uint8_t>(sig.signature[0]),
              static_cast<uint8_t>(sig.signature[1]),
              static_cast<uint8_t>(sig.signature[2]),
              static_cast<uint8_t>(sig.signature[3]),
              static_cast<uint8_t>(sig.signature[4]),
              static_cast<uint8_t>(sig.signature[5]),
              static_cast<uint8_t>(sig.signature[6]),
              static_cast<uint8_t>(sig.signature[7])
            );
          }

          case DW_FORM_exprloc: {
            std::string output;
            for (const auto& expr : dwarf.getAttributeValueExprLoc(attr)) {
              if (expr.lr_atom == DW_OP_addr) {
                output += folly::sformat(
                  "<OP_addr: {:#x}>,",
                  expr.lr_number
                );
              } else {
                output += folly::sformat(
                  "<{:#02x}:{}:{}:{}>,",
                  expr.lr_atom,
                  expr.lr_number,
                  expr.lr_number2,
                  expr.lr_offset
                );
              }
            }
            return folly::sformat("Location: [{}]", output);
          }

          case DW_FORM_block1:
          case DW_FORM_block2:
          case DW_FORM_block4:
          case DW_FORM_block: return "{BLOCK}";

          case DW_FORM_indirect: return "{INDIRECT}";
          case DW_FORM_sec_offset: return "{SECTION OFFSET}";
          default: return "{UNKNOWN}";
        }
      }();

      for (int i = 0; i < indent; ++i) {
        os << "  ";
      }
      os << "   **** " << attr_name << " (" << type << ") ==> "
         << attr_value << '\n';
      return true;
    }
  );

  recurse();
}

struct PrinterImpl : Printer {
  explicit PrinterImpl(const std::string& filename): m_filename{filename} {}
  void operator()(std::ostream& os,
                  std::size_t begin,
                  std::size_t end) const override {
    DwarfState dwarf{m_filename};

    // If a non-default begin parameter was specified, first iterate over all
    // the compilation units. Find the first compilation unit which at least
    // partially lies within the range given by the begin parameter. This is the
    // first compilation unit to begin printing from.
    folly::Optional<Dwarf_Off> last;
    if (begin > 0) {
      dwarf.forEachCompilationUnit(
        [&](Dwarf_Die cu) {
          const auto offset = dwarf.getDIEOffset(cu);
          if (offset <= begin) last = offset;
        }
      );
    }

    // Now iterate over all the compilation units again. Only actually print out
    // compilation units if they lie within the begin/end parameter range.
    dwarf.forEachCompilationUnit(
      [&](Dwarf_Die cu) {
        const auto offset = dwarf.getDIEOffset(cu);
        if ((!last || offset >= *last) && offset < end) {
          printDIE(
            os,
            dwarf,
            cu,
            // If this compilation unit entirely lies within the begin/end
            // range, specify a begin parameter of "0", which will stop
            // printDIE() from doing range checks (which is more efficient).
            (!last || (offset > *last)) ? 0 : begin,
            end
          );
        }
      }
    );

    os << std::flush;
  }
private:
  std::string m_filename;
};

////////////////////////////////////////////////////////////////////////////////

}

std::unique_ptr<TypeParser>
make_dwarf_type_parser(const std::string& filename) {
  return folly::make_unique<TypeParserImpl>(filename);
}

std::unique_ptr<Printer> make_dwarf_printer(const std::string& filename) {
  return folly::make_unique<PrinterImpl>(filename);
}

////////////////////////////////////////////////////////////////////////////////

}

#endif
