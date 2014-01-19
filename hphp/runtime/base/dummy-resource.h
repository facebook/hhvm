/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_DUMMY_RESOURCE_H_
#define incl_HPHP_DUMMY_RESOURCE_H_

#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * DummyResource is used in a number of places where the runtime wants
 * to cast a value to the resource type. Ideally, casting a non-resource
 * value to a resource would throw or produce null, but there are a few
 * places in the runtime and the extensions that would need to be updated
 * first to make that work.
 */
class DummyResource : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(DummyResource);
  CLASSNAME_IS("Unknown");
  DummyResource();
  String m_class_name;
  virtual const String& o_getClassNameHook() const;
  virtual bool isInvalid() const { return m_class_name.empty(); }
  void o_setResourceId(int64_t id) { o_id = id; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PHP_MAILPARSE_MIME_H_
