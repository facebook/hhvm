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
// TODO(#4489053) This should use hhvm constructs as well as be cleaned up

#ifndef incl_XDEBUG_VAR_H_
#define incl_XDEBUG_VAR_H_

#include "hphp/runtime/base/base-includes.h"

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

struct XDebugExporter {
  typedef smart::hash_map<void*, int, pointer_hash<void> > SmartPtrCtrMap;
  SmartPtrCtrMap counts; // Map of pointer -> # of times we've seen this object
  int level = 0;         // Current level we are at

  // These all must be provided
  uint32_t max_depth;         // Max depth to print for arrays/objects
  uint32_t max_children;      // Max children to print for arrays/objects
  uint32_t max_data;          // Max data to print for strings
  // Page to retrieve on the top level. Object/array members are split into
  // "pages" of size max_children. php5 xdebug has a page field for each
  // level, but it only ever sets the page for the top level
  uint32_t page;
};

// Exports the given variable into the returned node using the
// given exporter. If given, name, fullName, and facet should be freeable.
xdebug_xml_node* xdebug_var_export_xml_node(const char* name,
                                            const char* fullName,
                                            const char* facet,
                                            const Variant& var,
                                            XDebugExporter& exporter);
// Variable type
enum class XDebugVarType {
  Normal = 0x00,
  Static = 0x01,
  Constant = 0x02
};

// Given a variable with the given name, returns an xml representation using
// the given exporter
xdebug_xml_node* xdebug_get_value_xml_node(const char* name,
                                           const Variant& val,
                                           XDebugVarType type,
                                           XDebugExporter& exporter);

// Lookup a PHP symbol based on the current frame. Supports a limited parsing
// of symbol names.
Variant xdebug_get_php_symbol(ActRec* ar, StringData* name);

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_XDEBUG_VAR_H */
