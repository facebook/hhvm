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
// TODO(#3704) This should use hhvm constructs as well as be cleaned up

#ifndef incl_XDEBUG_VAR_H_
#define incl_XDEBUG_VAR_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/req-containers.h"

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////
// Errors

StaticString xdebug_error_type(int errnum);

////////////////////////////////////////////////////////////////////////////////
// Variable Exporting

struct XDebugExporter {
  /* Resets the state of the exporter.  Called before each export. */
  void reset(uint32_t new_level) {
    seen.clear();
    level = new_level;
  }

  /* Set of seen ObjectData and ArrayData pointers. */
  req::hash_set<void*, pointer_hash<void>> seen;

  /* Current level we are at. */
  uint32_t level{0};

  /* These must all be provided. */
  uint32_t max_depth;         // Max depth to print for arrays/objects.
  uint32_t max_children;      // Max children to print for arrays/objects.
  uint32_t max_data;          // Max data to print for strings.

  /*
   * Page to retrieve on the top level.  Object/array members are split into
   * "pages" of size max_children.  php5 xdebug has a page field for each level,
   * but it only ever sets the page for the top level.
   */
  uint32_t page;

  /* Only set by xdebug_find_name(). */
  bool no_decoration{false};

  /* Set by xdebug_debug_zval* */
  bool debug_zval{false};
};

/*
 * Exports the given variable into the returned node using the given exporter.
 * If given, name, fullName, and facet should be freeable.
 */
xdebug_xml_node* xdebug_var_export_xml_node(const char* name,
                                            const char* fullName,
                                            const char* facet,
                                            const Variant& var,
                                            XDebugExporter& exporter);

enum class XDebugVarType {
  Normal,
  Static,
  Constant,
};

String xdebug_get_zval_value_fancy(const Variant&, XDebugExporter&);
String xdebug_get_zval_value_ansi(const Variant&, XDebugExporter&);
String xdebug_get_zval_value_text(const Variant&, XDebugExporter&);

/*
 * Given a variable with the given name, returns an xml representation using the
 * given exporter.
 */
xdebug_xml_node* xdebug_get_value_xml_node(const char* name,
                                           const Variant& val,
                                           XDebugVarType type,
                                           XDebugExporter& exporter);

/*
 * Lookup a PHP symbol based on the current frame.  Supports a limited parsing
 * of symbol names.
 */
Variant xdebug_get_php_symbol(ActRec* ar, StringData* name);

///////////////////////////////////////////////////////////////////////////////
}

#endif
