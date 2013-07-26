/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_DUMMY_RESOURCE_H_
#define incl_HPHP_DUMMY_RESOURCE_H_

#include "hphp/runtime/base/complex_types.h"

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
  DECLARE_OBJECT_ALLOCATION(DummyResource);
  DummyResource();
  static StaticString s_class_name;
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }
  virtual bool isInvalid() const { return true; }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PHP_MAILPARSE_MIME_H_
