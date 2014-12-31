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

#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////
// PHP Errors

static const StaticString
  s_FATAL_ERROR("Fatal error"),
  s_CATCHABLE_FATAL_ERROR("Catchable fatal error"),
  s_WARNING("Warning"),
  s_PARSE_ERROR("Parse error"),
  s_NOTICE("Notice"),
  s_STRICT_STANDARDS("Strict standards"),
  s_DEPRECATED("Deprecated"),
  s_XDEBUG("Xdebug"),
  s_UNKNOWN_ERROR("Unknown error");

// Errors are generally passed around as errnum ints corresponding to an enum
// ErrorModes value
typedef ErrorConstants::ErrorModes ErrType;

// String name for the given error type, as defined by php5 xdebug in
// xdebug_var.c
const String xdebug_error_type(int errnum) {
  switch (static_cast<ErrType>(errnum)) {
    case ErrType::ERROR:
    case ErrType::CORE_ERROR:
    case ErrType::COMPILE_ERROR:
    case ErrType::USER_ERROR:
      return s_FATAL_ERROR;
    case ErrType::RECOVERABLE_ERROR:
      return s_CATCHABLE_FATAL_ERROR;
    case ErrType::WARNING:
    case ErrType::CORE_WARNING:
    case ErrType::COMPILE_WARNING:
    case ErrType::USER_WARNING:
      return s_WARNING;
    case ErrType::PARSE:
      return s_PARSE_ERROR;
    case ErrType::NOTICE:
    case ErrType::USER_NOTICE:
      return s_NOTICE;
    case ErrType::STRICT:
      return s_STRICT_STANDARDS;
    case ErrType::PHP_DEPRECATED:
    case ErrType::USER_DEPRECATED:
      return s_DEPRECATED;
    default:
      return s_UNKNOWN_ERROR;
  }
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
  // Grab the propeties on the object. o_toIterArray does this for us
  Class* cls = obj->getVMClass();
  Array props = obj->o_toIterArray(String(cls->name()->data(), CopyString));

  // Grab the  static properties from the class
  const Class::SProp* staticProperties = cls->staticProperties();
  const size_t nSProps = cls->numStaticProperties();
  for (Slot i = 0; i < nSProps; ++i) {
    auto const& prop = staticProperties[i];
    TypedValue* val = cls->getSPropData(i);
    if (val != nullptr) {
      props.set(String(prop.m_name->data(), CopyString), tvAsVariant(val));
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

    // If we've already seen this object, return
    if (exporter.counts[arr.get()]++ > 0) {
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
    exporter.counts[arr.get()]--;
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

    // If we've already seen this object, return
    if (exporter.counts[obj]++ > 0) {
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
    exporter.counts[(void*) obj]--;
  } else if (var.isResource()) {
    ResourceData* res = var.toResource().get();
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

xdebug_xml_node* xdebug_get_value_xml_node(const char* name,
                                           const Variant& val,
                                           XDebugVarType type
                                            /* = XDebugVarType::Normal */,
                                           XDebugExporter& exporter) {
  // Ensure there all state is cleared in the exporter. This allows the same
  // exporter to be used in multiple exports.
  exporter.reset();

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
