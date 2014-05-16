/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_CUSTOM_ELEMENT_H_
#define incl_HPHP_ZEND_CUSTOM_ELEMENT_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

/**
 * A resource class for wrapping arbitrary data stored to arrays by Zend compat
 * extensions. Allows an arbitrary destructor to be called when the resource is
 * freed.
 */
class ZendCustomElement : public ResourceData {
public:
  typedef void (*DtorFunc)(void *pDest);

  ZendCustomElement(void * data, unsigned int data_size, void ** dest,
      DtorFunc destructor);

  CLASSNAME_IS("ZendCustomElement");
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  const void * data() const { return m_data; }
  void * data() { return m_data; }

  virtual ~ZendCustomElement();

private:
  DtorFunc m_destructor;
  void * m_data;
};

}
#endif
