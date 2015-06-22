/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_var.h"

#include <cstdint>

#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////
// PHP Errors

const StaticString
  s_FATAL_ERROR("Fatal error"),
  s_CATCHABLE_FATAL_ERROR("Catchable fatal error"),
  s_WARNING("Warning"),
  s_PARSE_ERROR("Parse error"),
  s_NOTICE("Notice"),
  s_STRICT_STANDARDS("Strict standards"),
  s_DEPRECATED("Deprecated"),
  s_XDEBUG("Xdebug"),
  s_UNKNOWN_ERROR("Unknown error");

// String name for the given error type, as defined by php5 xdebug in
// xdebug_var.c
StaticString xdebug_error_type(int errnum) {
  switch (static_cast<ErrorMode>(errnum)) {
  case ErrorMode::ERROR:
  case ErrorMode::CORE_ERROR:
  case ErrorMode::COMPILE_ERROR:
  case ErrorMode::USER_ERROR:
    return s_FATAL_ERROR;
  case ErrorMode::RECOVERABLE_ERROR:
    return s_CATCHABLE_FATAL_ERROR;
  case ErrorMode::WARNING:
  case ErrorMode::CORE_WARNING:
  case ErrorMode::COMPILE_WARNING:
  case ErrorMode::USER_WARNING:
    return s_WARNING;
  case ErrorMode::PARSE:
    return s_PARSE_ERROR;
  case ErrorMode::NOTICE:
  case ErrorMode::USER_NOTICE:
    return s_NOTICE;
  case ErrorMode::STRICT:
    return s_STRICT_STANDARDS;
  case ErrorMode::PHP_DEPRECATED:
  case ErrorMode::USER_DEPRECATED:
    return s_DEPRECATED;
  default:
    return s_UNKNOWN_ERROR;
  }
}

namespace {
///////////////////////////////////////////////////////////////////////////////
// Property demangling

enum class PropModifier {
  Public,
  Protected,
  Private,
};

struct PropInfo {
  PropModifier modifier;
  String prop_name;
  String cls_name;
};

const char* modifier_str(PropModifier mod) {
  switch (mod) {
  case PropModifier::Public:    return "public";
  case PropModifier::Protected: return "protected";
  case PropModifier::Private:   return "private";
  }
  not_reached();
  return nullptr;
}

PropInfo demangle_prop(const String& prop_name) {
  PropInfo info;

  // Needs to be at least 3 chars for private/protected properties.
  // There's two extra NUL chars, plus either a '*' for protected, or
  // the classname for private.
  if (prop_name.size() >= 3 && prop_name[0] == '\0') {
    if (prop_name[1] == '*') {
      info.modifier = PropModifier::Protected;
      info.cls_name = empty_string();
      info.prop_name = prop_name.substr(3);
    } else {
      auto const cls_end = prop_name.find('\0', 1);
      info.modifier = PropModifier::Private;
      info.cls_name = prop_name.substr(1, cls_end - 1);
      info.prop_name = prop_name.substr(cls_end + 1);
    }
  } else {
    info.modifier = PropModifier::Public;
    info.cls_name = empty_string();
    info.prop_name = prop_name;
  }
  return info;
}

void xdebug_format_file_link(StringBuffer& sb, const char* filename, int line) {
  auto& format = XDEBUG_GLOBAL(FileLinkFormat);

  auto const size = format.size();

  for (size_t i = 0; i < size; ++i) {
    auto const c = format[i];

    if (c != '%') {
      sb.append(c);
      continue;
    }

    ++i;

    if (i >= size) {
      break;
    }

    switch (format[i]) {
    case 'f':
      sb.append(filename);
      break;
    case 'l':
      sb.append(line);
      break;
    case '%':
      sb.append('%');
      break;
    default:
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * RAII helper for incrementing/decrementing indent levels.
 */
struct Indenter {
  explicit Indenter(XDebugExporter& exp): exp(exp) {
    exp.level++;
  }

  ~Indenter() {
    exp.level--;
  }

  XDebugExporter& exp;
};

/*
 * RAII helper for tracking seen arrays and objects.
 */
struct Tracker {
  explicit Tracker(XDebugExporter& exp, void* ptr): exp(exp), ptr(ptr) {
    auto const result = exp.seen.insert(ptr);
    seen = !result.second;
  }

  ~Tracker() {
    exp.seen.erase(ptr);
  }

  XDebugExporter& exp;
  void* ptr;
  bool seen;
};

///////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
// XML node printing routines

// Exports the given xml key value pair as an xml child of the given parent node
static void xdebug_array_element_export_xml_node(xdebug_xml_node& parent,
                                                 const char* parentName,
                                                 const Variant& key,
                                                 const Variant& val,
                                                 XDebugExporter& exporter) {
  String key_str = key.toString();
  Variant full_name = init_null();

  // Construct the full name
  StringBuffer buf;
  if (parentName != nullptr)  {
    if (key.isInteger()) {
      buf.printf("%s[%s]", parentName, key_str.data());
    } else {
      buf.printf("%s['%s']", parentName, key_str.data());
    }
    full_name = buf.detach();
  }
  const char* full_name_str = full_name.isNull() ?
    nullptr : full_name.toString().data();

  // Recursively add the child
  xdebug_xml_node* child = xdebug_var_export_xml_node(xdstrdup(key_str.data()),
                                                      xdstrdup(full_name_str),
                                                      nullptr,
                                                      val, exporter);
  xdebug_xml_add_child(&parent, child);
}

static void xdebug_object_element_export_xml_node(xdebug_xml_node& parent,
                                                  const char* parentName,
                                                  const ObjectData* obj,
                                                  const Variant& key,
                                                  const Variant& val,
                                                  XDebugExporter& exporter) {
  auto const prop_str = key.toString();
  auto const cls = obj->getVMClass();
  auto const cls_name = cls->name()->data();

  // Compute whether the property is static

  auto const sLookup = cls->getSProp(nullptr, prop_str.get());

  auto const is_static = sLookup.prop != nullptr;
  bool visible = is_static;
  bool accessible = sLookup.accessible;

  // If the property is not static, we know it's a member, but need to grab the
  // visibility
  if (!is_static) {
    auto const lookup = obj->getProp(nullptr, prop_str.get());
    visible = lookup.prop != nullptr;
    accessible = lookup.accessible;
  }

  // This is public if it is visible and accessible from the nullptr context
  auto const is_public = visible && accessible;

  // Compute the property name and full name
  const char* name;
  const char* full_name = nullptr;
  if (!val.isInteger()) {
    // Compute the property name
    if (is_public) {
      name = xdstrdup(prop_str.data());
    } else {
      name = xdebug_sprintf("*%s*%s", cls_name, prop_str.data());
    }

    // Compute the property full name if we have a parent name
    if (parentName != nullptr) {
      if (is_public && is_static) {
        full_name = xdebug_sprintf("%s::%s", parentName, prop_str.data());
      } else if (is_public && !is_static) {
        full_name = xdebug_sprintf("%s->%s", parentName, prop_str.data());
      } else if (!is_public && is_static) {
        full_name = xdebug_sprintf("%s::*%s*%s", parentName, "::",
                                   cls_name, prop_str.data());
      } else if (!is_public && !is_static) {
        full_name = xdebug_sprintf("%s->*%s*%s", parentName, "->",
                                   cls_name, prop_str.data());
      }
    }
  } else {
    // Compute the name + full name if we have a parent name
    name = xdebug_sprintf("%ld", key.toInt64());
    if (parentName != nullptr) {
      if (is_static) {
        full_name = xdebug_sprintf("%s::%ld", parentName, key.toInt64());
      } else {
        full_name = xdebug_sprintf("%s->%ld", parentName, key.toInt64());
      }
    }
  }

  // Compute the facet (static/non-static + public/private)
  const char* facet = xdebug_sprintf("%s%s",
                                     is_static ? "static " : "",
                                     is_public ? "public" : "private");

  // Recursively write the this property. The duplications are necessary due to
  // the xdebug xml api
  xdebug_xml_node* child = xdebug_var_export_xml_node(xdstrdup(name),
                                                      xdstrdup(full_name),
                                                      xdstrdup(facet),
                                                      val, exporter);
  xdebug_xml_add_child(&parent, child);
}

static char* prepare_variable_name(const char* name) {
  const char* prefix = (name[0] == '$' || name[0] == ':') ? "" : "$";
  char* tmp_name = xdebug_sprintf("%s%s", prefix, name);
  if (tmp_name[strlen(tmp_name) - 2] == ':' &&
      tmp_name[strlen(tmp_name) - 1] == ':') {
    tmp_name[strlen(tmp_name) - 2] = '\0';
  }
  return tmp_name;
}

// Return an array of prop name => prop val. This is taken (and modified) from
// ext_reflection.cpp
static Array get_object_props(ObjectData* obj) {
  // Grab the properties on the object. o_toIterArray does this for us.
  auto const cls = obj->getVMClass();
  auto const cls_name = cls->name();
  auto props = obj->o_toIterArray(String(const_cast<StringData*>(cls_name)),
                                  ObjectData::EraseRefs);

  // Grab the static properties from the class.
  auto const staticProperties = cls->staticProperties();
  auto const nSProps = cls->numStaticProperties();
  for (Slot i = 0; i < nSProps; ++i) {
    auto const& prop = staticProperties[i];
    auto val = cls->getSPropData(i);
    if (val != nullptr) {
      auto const prop_name = prop.m_name.get();
      props.set(String(const_cast<StringData*>(prop_name)), tvAsVariant(val));
    }
  }

  return props;
}

xdebug_xml_node* xdebug_var_export_xml_node(const char* name,
                                            const char* fullName,
                                            const char* facet,
                                            const Variant& var,
                                            XDebugExporter& exporter) {
  // Setup the node. Each const cast is necessary due to xml api
  xdebug_xml_node* node = xdebug_xml_node_init("property");
  if (name != nullptr) {
    xdebug_xml_add_attribute_ex(node, "name", const_cast<char*>(name), 0, 1);
  }
  if (fullName != nullptr) {
    xdebug_xml_add_attribute_ex(node, "fullname", const_cast<char*>(fullName),
                                0, 1);
  }
  if (facet != nullptr) {
    xdebug_xml_add_attribute_ex(node, "facet", const_cast<char*>(facet), 0, 1);
  }
  xdebug_xml_add_attribute_ex(node, "address",
                              xdebug_sprintf("%ld", (long) &var), 0, 1);

  // Case on the type for the rest
  if (var.isBoolean()) {
    xdebug_xml_add_attribute(node, "type", "bool");
    xdebug_xml_add_text(node, xdebug_sprintf("%d",  var.toBoolean()));
  } else if (var.isNull()) {
    xdebug_xml_add_attribute(node, "type", "null");
  } else if (var.isInteger()) {
    xdebug_xml_add_attribute(node, "type", "int");
    xdebug_xml_add_text(node, xdebug_sprintf("%ld", var.toInt64()));
  } else if (var.isDouble()) {
    xdebug_xml_add_attribute(node, "type", "float");
    xdebug_xml_add_text(node, xdebug_sprintf("%lG", var.toDouble()));
  } else if (var.isString()) {
    // Add the type and the original size
    String str = var.toString();
    xdebug_xml_add_attribute(node, "type", "string");
    xdebug_xml_add_attribute_ex(node, "size",
                                xdebug_sprintf("%d", str.size()), 0, 1);

    // Possibly shrink the string, then add it to the node
    if (exporter.max_data != 0 && str.size() > exporter.max_data) {
      str = str.substr(0, exporter.max_data);
    }
    xdebug_xml_add_text_encodel(node, xdstrdup(str.data()), str.size());
  } else if (var.isArray()) {
    Array arr = var.toArray();
    xdebug_xml_add_attribute(node, "type", "array");
    xdebug_xml_add_attribute(node, "children",
                             const_cast<char*>(arr.size() > 0 ? "1" : "0"));

    Tracker track(exporter, arr.get());

    // If we've already seen this object, return
    if (track.seen) {
      xdebug_xml_add_attribute(node, "recursive", "1");
      return node;
    }

    // Write the # of children then short-circuit if we are too deep
    xdebug_xml_add_attribute_ex(node, "numchildren",
                                xdebug_sprintf("%d", arr.size()), 0, 1);
    if (exporter.level++ >= exporter.max_depth) {
      return node;
    }

    // Compute the page and the start/end indices
    // Note that php xdebug doesn't support pages except for at the top level
    uint32_t page = exporter.level == 1 ? exporter.page : 0;
    uint32_t start = page * exporter.max_children;
    uint32_t end = (page + 1) * exporter.max_children;
    xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", page), 0, 1);
    xdebug_xml_add_attribute_ex(node, "pagesize",
                                xdebug_sprintf("%d", exporter.max_children),
                                0, 1);
    // Add each child
    ArrayIter iter(arr);
    iter.setPos(start);
    for (uint32_t i = start; i < end && iter; i++, ++iter) {
      xdebug_array_element_export_xml_node(*node, name,
                                           iter.first(),
                                           iter.second(),
                                           exporter);
    }

    // Done at this level
    exporter.level--;
  } else if (var.isObject()) {
    // TODO(#3704) This could be merged into the above array code. For now,
    // it's separate as this was pulled originally from xdebug
    ObjectData* obj = var.toObject().get();
    Class* cls = obj->getVMClass();
    Array props = get_object_props(obj);

    // Add object info
    xdebug_xml_add_attribute(node, "type", "object");
    xdebug_xml_add_attribute_ex(node, "classname",
                                xdstrdup(cls->name()->data()), 0, 1);
    xdebug_xml_add_attribute(node, "children",
                             const_cast<char*>(props.size() ? "1" : "0"));

    Tracker track(exporter, obj);

    // If we've already seen this object, return
    if (track.seen) {
      xdebug_xml_add_attribute(node, "recursive", "1");
      return node;
    }

    // Add the # of props then short circuit if we are too deep
    xdebug_xml_add_attribute_ex(node, "numchildren",
                                xdebug_sprintf("%d", props.size()), 0, 1);
    if (exporter.level++ >= exporter.max_depth) {
      return node;
    }

    // Compute the page and the start/end indices
    // Note that php xdebug doesn't support pages except for at the top level
    uint32_t page = exporter.level == 1 ? exporter.page : 0;
    uint32_t start = page * exporter.max_children;
    uint32_t end = (page + 1) * exporter.max_children;
    xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", page), 0, 1);
    xdebug_xml_add_attribute_ex(node, "pagesize",
                                xdebug_sprintf("%d", exporter.max_children),
                                0, 1);

    // Add each property
    ArrayIter iter(props);
    iter.setPos(start);
    for (uint32_t i = start; i < end && iter; i++, ++iter) {
      xdebug_object_element_export_xml_node(*node, name, obj,
                                            iter.first(),
                                            iter.second(),
                                            exporter);
    }

    // Done at this level
    exporter.level--;
  } else if (var.isResource()) {
    auto res = var.toResource();
    xdebug_xml_add_attribute(node, "type", "resource");
    const char* text = xdebug_sprintf("resource id='%ld' type='%s'",
                                      res->o_getId(),
                                      res->o_getResourceName().data());
    xdebug_xml_add_text(node, const_cast<char*>(text));
  } else {
    xdebug_xml_add_attribute(node, "type", "null");
  }
  return node;
}

namespace {
///////////////////////////////////////////////////////////////////////////////

/* For ansi printing. */

#define ANSI_COLOR_POINTER       (ansi ? "\x1b[0m" : "")
#define ANSI_COLOR_BOOL          (ansi ? "\x1b[35m" : "")
#define ANSI_COLOR_LONG          (ansi ? "\x1b[32m" : "")
#define ANSI_COLOR_NULL          (ansi ? "\x1b[34m" : "")
#define ANSI_COLOR_DOUBLE        (ansi ? "\x1b[33m" : "")
#define ANSI_COLOR_STRING        (ansi ? "\x1b[31m" : "")
#define ANSI_COLOR_EMPTY         (ansi ? "\x1b[30m" : "")
#define ANSI_COLOR_ARRAY         (ansi ? "\x1b[33m" : "")
#define ANSI_COLOR_OBJECT        (ansi ? "\x1b[31m" : "")
#define ANSI_COLOR_RESOURCE      (ansi ? "\x1b[36m" : "")
#define ANSI_COLOR_MODIFIER      (ansi ? "\x1b[32m" : "")
#define ANSI_COLOR_RESET         (ansi ? "\x1b[0m" : "")
#define ANSI_COLOR_BOLD          (ansi ? "\x1b[1m" : "")
#define ANSI_COLOR_BOLD_OFF      (ansi ? "\x1b[22m" : "")

/* For fancy printing. */
#define COLOR_POINTER   "#888a85"
#define COLOR_BOOL      "#75507b"
#define COLOR_LONG      "#4e9a06"
#define COLOR_NULL      "#3465a4"
#define COLOR_DOUBLE    "#f57900"
#define COLOR_STRING    "#cc0000"
#define COLOR_EMPTY     "#888a85"
#define COLOR_ARRAY     "#ce5c00"
#define COLOR_OBJECT    "#8f5902"
#define COLOR_RESOURCE  "#2e3436"

const StaticString
  s_ansi_esc("\0"),
  s_text_esc("'\\\0..\37"),
  s_array_str_esc("'\0");

///////////////////////////////////////////////////////////////////////////////

void xdebug_var_export_text_ansi(
  StringBuffer& sb,
  const Variant& v,
  bool ansi,
  XDebugExporter& exporter
) {
  sb.printf("%*s", (exporter.level * 2) - 2, "");

  if (exporter.debug_zval) {
    sb.printf(
      "(refcount=%d, is_ref=%d)=",
      v.getRefCount(),
      v.getType() == KindOfRef
    );
  }

  switch (v.getType()) {
  case KindOfUninit:
    /* fallthrough */
  case KindOfNull:
    sb.printf(
      "%s%sNULL%s%s",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_NULL,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_RESET
    );
    break;
  case KindOfBoolean:
    sb.printf(
      "%sbool%s(%s%s%s)",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_BOOL,
      v.toBooleanVal() ? "true" : "false",
      ANSI_COLOR_RESET
    );
    break;
  case KindOfInt64:
    sb.printf(
      "%sint%s(%s%ld%s)",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_LONG,
      v.toInt64Val(),
      ANSI_COLOR_RESET
    );
    break;
  case KindOfDouble:
    // XXX: This should respect ini_get("precision").
    sb.printf(
      "%sdouble%s(%s%f%s)",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_DOUBLE,
      v.toDoubleVal(),
      ANSI_COLOR_RESET
    );
    break;
  case KindOfStaticString:
    /* fallthrough */
  case KindOfString: {
    auto const charlist = ansi ? s_ansi_esc : s_text_esc;
    auto const& str = v.toCStrRef();
    auto const esc_str = HHVM_FN(addcslashes)(str, charlist);

    if (exporter.no_decoration) {
      sb.append(esc_str);
      break;
    }

    sb.printf(
      "%sstring%s(%s%d%s) \"%s",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_LONG,
      str.size(),
      ANSI_COLOR_RESET,
      ANSI_COLOR_STRING
    );

    if (str.size() <= exporter.max_data) {
      sb.append(esc_str);
    } else {
      sb.append(esc_str.substr(0, exporter.max_data));
    }

    sb.append(ANSI_COLOR_RESET);
    sb.append('"');
    if (str.size() > exporter.max_data) {
      sb.append("...");
    }
    break;
  }
  case KindOfResource: {
    auto const& res = v.toCResRef();

    sb.printf(
      "%sresource%s(%s%" PRId32 "%s) of type (%s)",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_RESOURCE,
      res->o_getId(),
      ANSI_COLOR_RESET,
      res->o_getClassName().c_str()
    );
    break;
  }
  case KindOfArray: {
    auto const& arr = v.toCArrRef();

    sb.printf("%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);

    Tracker track(exporter, arr.get());
    if (track.seen) {
      break;
    }

    sb.printf("(%s%zd%s) {\n", ANSI_COLOR_LONG, arr.size(), ANSI_COLOR_RESET);

    if (exporter.level <= exporter.max_depth) {
      uint32_t elem_count = 0;

      for (ArrayIter iter(arr); iter; ++iter) {
        if (elem_count == exporter.max_children) {
          sb.printf("\n%*s(more elements)...\n", (exporter.level * 2), "");
          break;
        }
        ++elem_count;
        sb.printf("%*s", exporter.level * 2, "");

        auto first = *iter.first().asTypedValue();
        switch (first.m_type) {
        case KindOfInt64:
          sb.printf(
            "[%ld] %s=>%s\n",
            first.m_data.num,
            ANSI_COLOR_POINTER,
            ANSI_COLOR_RESET
          );
          break;
        case KindOfStaticString:
        case KindOfString: {
          auto const key_str = String{first.m_data.pstr};
          auto const esc_str = HHVM_FN(addcslashes)(
            key_str,
            s_array_str_esc
          );

          // Matches the php5 extension in that this case doesn't use ANSI
          // color codes...
          sb.printf("'%s' =>\n", esc_str.c_str());
          break;
        }
        default:
          not_reached();
        }

        Indenter ind(exporter);
        xdebug_var_export_text_ansi(sb, iter.secondRef(), ansi, exporter);
      }
    } else {
      sb.printf("%*s...\n", exporter.level * 2, "");
    }

    sb.printf("%*s}", (exporter.level * 2) - 2, "");
    break;
  }
  case KindOfObject: {
    auto const& obj = v.toCObjRef();

    // XXX: This changes the refcounts of the object's properties.  Matters if
    // exporter.debug_zval is set.
    auto const obj_arr = obj.toArray();

    Tracker track(exporter, obj.get());
    if (track.seen) {
      sb.printf("%*s...\n", exporter.level * 2, "");
      break;
    }

    sb.printf(
      "%sclass%s %s%s%s#%d (%s%zd%s) {\n",
      ANSI_COLOR_BOLD,
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_OBJECT,
      obj->getClassName().c_str(),
      ANSI_COLOR_RESET,
      obj->getId(),
      ANSI_COLOR_LONG,
      obj_arr.size(),
      ANSI_COLOR_RESET
    );

    if (exporter.level > exporter.max_depth) {
      sb.printf("%*s...\n", exporter.level * 2, "");
      break;
    }

    uint32_t elem_count = 0;
    for (ArrayIter iter(obj_arr); iter; ++iter) {
      if (elem_count == exporter.max_children) {
        sb.printf("\n%*s(more elements)...\n", exporter.level * 2, "");
        break;
      }

      ++elem_count;
      sb.printf("%*s", exporter.level * 2, "");

      auto first = *iter.first().asTypedValue();

      auto prop_name = IS_STRING_TYPE(first.m_type)
        ? String{first.m_data.pstr}
        : String{first.m_data.num};

      auto info = demangle_prop(prop_name);

      sb.printf(
        "%s%s%s%s%s $",
        ANSI_COLOR_MODIFIER,
        ANSI_COLOR_BOLD,
        modifier_str(info.modifier),
        ANSI_COLOR_BOLD_OFF,
        ANSI_COLOR_RESET
      );

      if (first.m_type == KindOfInt64) {
        sb.printf("{%" PRId64 "}", first.m_data.num);
      } else {
        sb.append(info.prop_name);
      }
      sb.printf(" %s=>%s\n", ANSI_COLOR_POINTER, ANSI_COLOR_RESET);

      Indenter ind(exporter);
      xdebug_var_export_text_ansi(sb, iter.secondRef(), ansi, exporter);
    }
    sb.printf("%*s}", (exporter.level * 2) - 2, "");
    break;
  }
  default:
    not_reached();
  }

  sb.append('\n');
}

String xdebug_get_zval_value_text_ansi(
  const Variant& v,
  bool ansi,
  XDebugExporter& exporter
) {
  exporter.reset(1);

  StringBuffer sb;

  if (XDEBUG_GLOBAL(OverloadVarDump) > 1) {
    sb.printf(
      "%s%s%s:%s%d%s:\n",
      ANSI_COLOR_BOLD,
      g_context->getContainingFileName()->data(),
      ANSI_COLOR_BOLD_OFF,
      ANSI_COLOR_BOLD,
      g_context->getLine(),
      ANSI_COLOR_BOLD_OFF
    );
  }

  xdebug_var_export_text_ansi(sb, v, ansi, exporter);
  return sb.detach();
}

void xdebug_var_export_fancy(
  StringBuffer& sb,
  const Variant& v,
  XDebugExporter& exporter
) {
  if (exporter.debug_zval) {
    sb.printf(
      "<i>(refcount=%d, is_ref=%d)</i>,",
      v.getRefCount(),
      v.getType() == KindOfRef
    );
  } else if (v.getType() == KindOfRef) {
    sb.append("&amp;");
  }

  switch (v.getType()) {
  case KindOfUninit:
    /* fallthrough */
  case KindOfNull:
    sb.printf("<font color='%s'>null</font>", COLOR_NULL);
    break;
  case KindOfBoolean:
    sb.printf(
      "<small>boolean</small> <font color='%s'>%s</font>",
      COLOR_BOOL,
      v.toBooleanVal() ? "true" : "false"
    );
    break;
  case KindOfInt64:
    sb.printf(
      "<small>int</small> <font color='%s'>%" PRId64 "</font>",
      COLOR_LONG,
      v.toInt64Val()
    );
    break;
  case KindOfDouble:
    // XXX: Should respect ini_get("precision").
    sb.printf(
      "<small>float</small> <font color='%s'>%f</font>",
      COLOR_DOUBLE,
      v.toDoubleVal()
    );
    break;
  case KindOfStaticString:
    /* fallthrough */
  case KindOfString: {
    auto const& str = v.toCStrRef();

    sb.printf("<small>string</small> <font color='%s'>'", COLOR_STRING);

    auto const xml_str = xdebug_xmlize(str.data(), str.size());
    if (str.size() <= exporter.max_data) {
      sb.append(xml_str);
      sb.append("'</font>");
    } else {
      sb.append(xml_str.substr(0, exporter.max_data));
      sb.append("'...</font>");
    }
    sb.printf(" <i>(length=%d)</i>", str.size());
    break;
  }
  case KindOfArray: {
    auto const& arr = v.toCArrRef();

    sb.printf("\n%*s", (exporter.level - 1) * 4, "");

    Tracker track(exporter, arr.get());
    if (track.seen) {
      sb.append("<i>&</i><b>array</b>\n");
      break;
    }

    sb.printf("<b>array</b> <i>(size=%zd)</i>\n", arr.size());

    if (exporter.level > exporter.max_depth) {
      sb.printf("%*s...\n", (exporter.level * 4) - 2, "");
      break;
    }

    if (arr.empty()) {
      sb.printf("%*s", (exporter.level * 4) - 2, "");
      sb.printf("<i><font color='%s'>empty</font></i>\n", COLOR_EMPTY);
      break;
    }

    uint32_t elem_count = 0;
    for (ArrayIter iter(arr); iter; ++iter) {
      if (elem_count == exporter.max_children) {
        sb.printf("%*s", (exporter.level * 4) - 2, "");
        sb.append("<i>more elements...</i>\n");
        break;
      }

      ++elem_count;
      sb.printf("%*s", (exporter.level * 4) - 2, "");

      auto first = *iter.first().asTypedValue();

      switch (first.m_type) {
      case KindOfInt64:
        sb.append(first.m_data.num);
        break;
      case KindOfStaticString:
      case KindOfString: {
        auto const str = first.m_data.pstr;
        sb.append('\'');
        sb.append(xdebug_xmlize(str->data(), str->size()));
        sb.append('\'');
        break;
      }
      default:
        not_reached();
      }

      sb.printf(" <font color='%s'>=&gt;</font> ", COLOR_POINTER);

      Indenter ind(exporter);
      xdebug_var_export_fancy(sb, iter.secondRef(), exporter);
    }
    break;
  }
  case KindOfObject: {
    auto const& obj = v.toCObjRef();

    sb.printf("\n%*s", (exporter.level - 1) * 4, "");

    Tracker track(exporter, obj.get());
    if (track.seen) {
      sb.append("<i>&</i>");
    }

    sb.printf(
      "<b>object</b>(<i>%s</i>)[<i>%d</i>]\n",
      obj->getClassName().c_str(),
      obj->getId()
    );

    if (track.seen) {
      break;
    }

    if (exporter.level > exporter.max_depth) {
      sb.printf("%*s...\n", (exporter.level * 4) - 2, "");
      break;
    }

    uint32_t elem_count = 0;
    // XXX: This changes the refcounts of the object's properties.  Matters if
    // exporter.debug_zval is set.
    for (ArrayIter iter(obj.toArray()); iter; ++iter) {
      if (elem_count == exporter.max_children) {
        sb.printf("%*s", (exporter.level * 4) - 2, "");
        sb.append("<i>more elements...</i>\n");
        break;
      }

      ++elem_count;
      sb.printf("%*s", (exporter.level * 4) - 2, "");

      auto first = *iter.first().asTypedValue();

      if (first.m_type == KindOfInt64) {
        sb.printf(
          "<i>public</i> %" PRId64 " <font =color='%s'>=&gt;</font> ",
          first.m_data.num,
          COLOR_POINTER
        );
        xdebug_var_export_fancy(sb, iter.secondRef(), exporter);
        continue;
      }

      auto prop_name = IS_STRING_TYPE(first.m_type)
        ? String{first.m_data.pstr}
      : empty_string();

      auto info = demangle_prop(prop_name);

      sb.printf(
        "<i>%s</i> '%s' ",
        modifier_str(info.modifier),
        info.prop_name.c_str()
      );

      // Only print the class if the property is private and declared in a
      // parent class.
      if (info.modifier == PropModifier::Private &&
          !info.cls_name.same(obj->getClassName())) {
        sb.printf("<small>(%s)</small> ", info.cls_name.c_str());
      }

      sb.printf("<font color='%s'>=&gt;</font> ", COLOR_POINTER);
      Indenter ind(exporter);
      xdebug_var_export_fancy(sb, iter.secondRef(), exporter);
    }
    break;
  }
  default:
    not_reached();
  }

  if (v.getType() != KindOfArray && v.getType() != KindOfObject) {
    sb.append('\n');
  }
}

///////////////////////////////////////////////////////////////////////////////
}

String xdebug_get_zval_value_ansi(const Variant& v, XDebugExporter& exporter) {
  return xdebug_get_zval_value_text_ansi(v, true, exporter);
}

String xdebug_get_zval_value_text(const Variant& v, XDebugExporter& exporter) {
  return xdebug_get_zval_value_text_ansi(v, false, exporter);
}

String xdebug_get_zval_value_fancy(const Variant& v, XDebugExporter& exporter) {
  exporter.reset(1);

  StringBuffer sb;

  sb.append("<pre class='xdebug-var-dump' dir='ltr'>");

  if (XDEBUG_GLOBAL(OverloadVarDump) > 1) {
    auto filename = g_context->getContainingFileName()->data();
    auto line = g_context->getLine();

    if (!XDEBUG_GLOBAL(FileLinkFormat).empty()) {
      xdebug_format_file_link(sb, filename, line);
    } else {
      sb.printf("\n<small>%s:%d:</small>", filename, line);
    }
  }

  xdebug_var_export_fancy(sb, v, exporter);

  sb.append("</pre>");

  return sb.detach();
}

xdebug_xml_node* xdebug_get_value_xml_node(
  const char* name,
  const Variant& val,
  XDebugVarType type,
  XDebugExporter& exporter
) {
  exporter.reset(0);

  // Compute the short and full name of the passed value
  char* short_name = nullptr;
  char* full_name = nullptr;
  if (name) {
    switch (type) {
      case XDebugVarType::Normal: {
        char* tmp_name = prepare_variable_name(name);
        short_name = xdstrdup(tmp_name);
        full_name = xdstrdup(tmp_name);
        xdfree(tmp_name);
        break;
      }
      case XDebugVarType::Static:
        short_name = xdebug_sprintf("::%s", name);
        full_name =  xdebug_sprintf("::%s", name);
        break;
      case XDebugVarType::Constant:
        short_name = xdstrdup(name);
        full_name =  xdstrdup(name);
        break;
      default:
        throw Exception("Invalid variable type");
    }
  }

  // Recursively construct the xml
  return xdebug_var_export_xml_node(short_name, full_name, nullptr,
                                    val, exporter);
}

///////////////////////////////////////////////////////////////////////////////
}
