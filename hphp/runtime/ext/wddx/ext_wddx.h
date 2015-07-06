/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_WDDX_H_
#define incl_HPHP_EXT_WDDX_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/string-buffer.h"

namespace HPHP {

class WddxPacket: public ResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(WddxPacket);

  WddxPacket(const Variant& comment, bool manualPacket, bool sVar);

  CLASSNAME_IS("WddxPacket");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }


  bool add_var(const String& varName, bool hasVarTag);
  String packet_end();
  bool serialize_value(const Variant& varVariant);
  bool recursiveAddVar(const String& varName, const Variant& varVariant,
                       bool hasVarTag );

 private:
  String getWddxEncoded(const String& varType,
                        const String& varValue,
                        const String& varName,
                        bool hasVarTag);

  String wrapValue(const String& start,
                   const String& end,
                   const String& varValue,
                   const String& varName,
                   bool hasVarTag);

  StringBuffer m_packetString;
  bool m_packetClosed;
  bool m_manualPacketCreation;
};

///////////////////////////////////////////////////////////////////////////////
// helper

void find_var_recursive(const TypedValue* tv,
                        const req::ptr<WddxPacket>& wddxPacket);

}

#endif
