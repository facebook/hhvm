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
// TODO(#4489053) This should be refactored after xml api is refactored.
// TODO(#4489053) Only the xml code has been pulled in for now

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_var.h"

#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////
// Dynamic symbol lookup

enum class SymbolType {
  StaticRoot = 0,
  StaticProp,
  Root,
  ArrayIndexAssoc,
  ArrayIndexNum,
  ObjProp
};

// Given a typed value tv return the associated class object
static Class* xdebug_get_sym_class(const Variant& var) {
  if (var.is(KindOfObject)) return var.toObject().get()->getVMClass();

  return nullptr;
}

// Find a symbol given by string key of the specified symbol type. Context
// information can be specified through ctx, sym, and ar.
//
// StaticRoot, StaticProp
//   * Search for a static property given by key in class ctx
//   * A class name may be encoded with the property as *classname*propname
//     to indicate a class that should be used as the visibility context for
//     loading the property
static Variant xdebug_lookup_symbol(SymbolType type, String key, Class*& ctx,
                                    Variant& sym, ActRec* ar) {
  char* name = key.get()->mutableData();
  char* end = key.get()->mutableData() + key.size();
  assert(name != end);

  switch (type) {
    case SymbolType::StaticRoot:
    case SymbolType::StaticProp: {
      TypedValue* ret = nullptr;
      if (!ctx) return uninit_null();

      Class* newCtx = nullptr;
      char* secStar;
      bool vis, acc;
      if ((ret = ctx->getSProp(ctx, key.get(), vis, acc))) {
        return tvAsVariant(ret);
      }

      for (secStar = name + 1; secStar != end && *secStar != '*'; ++secStar);
      if (secStar != end && *name == '*' && *secStar == '*') {
        String clsKey(name + 1, secStar - name - 1, CopyStringMode());
        String newKey(secStar + 1, end - secStar - 1, CopyStringMode());
        newCtx = Unit::lookupClass(clsKey.get());
        if (newCtx && (ret = ctx->getSProp(newCtx, newKey.get(),
                                              vis, acc))) {
          return tvAsVariant(ret);
        }
      }
      return uninit_null();
    }
    break;

    case SymbolType::Root: {
      const Func* func = ar->func();

      if (key.size() == 4 && strncmp(name, "this", 4) == 0) {
        return ar->hasThis() ? ar->getThis() : nullptr;
      }

      Id localId = func->lookupVarId(key.get());

      if (localId != kInvalidId) {
        TypedValue* tv = frame_local(ar, localId);
        return tv ? tvAsVariant(tv) : uninit_null();
      }

      Class* tmp = Unit::lookupClass(key.get());

      if (tmp) ctx = tmp;
      return uninit_null();
    }
    break;

    case SymbolType::ArrayIndexAssoc: {
      return  sym.isArray()  ? sym.asArrRef().rvalAt(key) :
              sym.isObject() ? sym.asObjRef().o_get(key, false)
                             : uninit_null();
    }

    case SymbolType::ArrayIndexNum: {
      int64_t iKey = key.toInt64();

      return  sym.isArray()  ? sym.asArrRef().rvalAt(iKey)
                             : uninit_null();
    }

    case SymbolType::ObjProp: {
      char* secStar;
      if (!sym.is(KindOfObject)) return uninit_null();

      Object obj = sym.toObject();
      Variant v = obj->o_get(key, false);
      if(!v.isNull()) return v;

      for (secStar = name + 1; secStar != end && *secStar != '*'; ++secStar);
      if (secStar != end && *name == '*' && *secStar == '*') {
        String clsKey(name + 1, secStar - name - 1, CopyStringMode());
        String newKey(secStar + 1, end - secStar - 1, CopyStringMode());
        v = obj.o_get(key, false, clsKey);
      }

      return v;
    }
  }

  not_reached();
}

Variant xdebug_get_php_symbol(ActRec* ar, StringData* name) {
  int state;
  auto slice = name->slice();
  const char *p = slice.begin();
  const char *end = slice.end();
  const char *keyword = nullptr;
  const char *keyword_end = nullptr;
  Class* ctx = nullptr;
  Variant sym;
  String key(name->size(), ReserveStringMode());
  char quotechar = 0;
  SymbolType type = SymbolType::Root;

  StringData* sd = key.get();
  char* keyBuf = sd->mutableData();

  for (; p != end; ++p) {
    switch (state) {
      case 0:
        if (*p == '$') {
          keyword = p + 1;
          break;
        }
        // special tricks
        if (*p == ':') {
          keyword = p;
          state = 7;
          break;
        }
        keyword = p;
        state = 1;
        // fallthrough
      case 1:
        if (*p == '[') {
          keyword_end = p;
          if (keyword) {
            memcpy(keyBuf, keyword, keyword_end - keyword);
            sd->setSize(keyword_end - keyword);
            sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
            ctx = nullptr;
            keyword = nullptr;
          }
          state = 3;
        } else if (*p == '-') {
          keyword_end = p;
          if (keyword) {
            memcpy(keyBuf, keyword, keyword_end - keyword);
            sd->setSize(keyword_end - keyword);
            sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
            ctx = xdebug_get_sym_class(sym);
            keyword = nullptr;
          }
          state = 2;
          type = SymbolType::ObjProp;
        } else if (*p == ':') {
          keyword_end = p;
          if (keyword) {
            memcpy(keyBuf, keyword, keyword_end - keyword);
            sd->setSize(keyword_end - keyword);
            // XXX: this call is going to set ctx
            sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
            keyword = nullptr;
          }
          state = 8;
          type = SymbolType::StaticProp;
        }
        break;
      case 2:
        if (*p != '>') {
          keyword = p;
          state = 1;
        }
        break;
      case 8:
        if (*p != ':') {
          keyword = p;
          state = 1;
        }
        break;
      // Parsing in [...]
      case 3:
        // Associative arrays
        if (*p == '\'' || *p == '"') {
          state = 4;
          keyword = p + 1;
          quotechar = *p;
          type = SymbolType::ArrayIndexAssoc;
        }
        // Numerical index
        if (*p >= '0' && *p <= '9') {
          state = 6;
          keyword = p;
          type = SymbolType::ArrayIndexNum;
        }
        // Numerical index starting with a -
        if (*p == '-') {
          state = 9;
          keyword = p;
        }
        break;
      // Numerical index starting with a -
      case 9:
        if (*p >= '0' && *p <= '9') {
          state = 6;
          type = SymbolType::ArrayIndexNum;
        }
        break;
      case 4:
        if (*p == quotechar) {
          quotechar = 0;
          state = 5;
          keyword_end = p;
          memcpy(keyBuf, keyword, keyword_end - keyword);
          sd->setSize(keyword_end - keyword);
          sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
          ctx = xdebug_get_sym_class(sym);
          keyword = nullptr;
        }
        break;
      case 5:
        if (*p == ']') {
          state = 1;
        }
        break;
      case 6:
        if (*p == ']') {
          state = 1;
          keyword_end = p;
          memcpy(keyBuf, keyword, keyword_end - keyword);
          sd->setSize(keyword_end - keyword);
          sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
          ctx = xdebug_get_sym_class(sym);
          keyword = nullptr;
        }
        break;
      // special cases, started with a ":"
      case 7:
        if (*p == ':') {
          state = 1;
          keyword_end = p;

          // static class properties
          if (keyword_end + 1 != end && strncmp(keyword, "::", 2) == 0) {
            ctx = ar->hasClass() ? ar->getClass() :
                  ar->hasThis()  ? ar->getThis()->getVMClass() :
                  nullptr;

            sym = uninit_null();
            keyword = p + 1;
            type = SymbolType::StaticRoot;
          } else {
            keyword = nullptr;
          }
        }
        break;
    }
  }
  if (keyword) {
    memcpy(keyBuf, keyword, p - keyword);
    sd->setSize(p - keyword);
    sym = xdebug_lookup_symbol(type, key, ctx, sym, ar);
  }
  return sym;
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
  String prop_str = key.toString();
  const Class* cls = obj->getVMClass();
  const char* cls_name = cls->name()->data();

  // Compute whether the properity is static
  bool visible, accessible;
  bool is_static = cls->getSProp(nullptr, prop_str.get(),
                                 visible, accessible) != nullptr;

  // If the property is not static, we know it's a member, but need to grab the
  // visibility
  if (!is_static) {
    bool unset;
    obj->getProp(nullptr, prop_str.get(), visible, accessible, unset);
  }

  // This is public if it is visible and accessible from the nullptr context
  bool is_public = visible && accessible;

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
    uint32_t page = exporter.level == 0 ? exporter.page : 0;
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
    // TODO(#4489053) This could be merged into the above array code. For now,
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
