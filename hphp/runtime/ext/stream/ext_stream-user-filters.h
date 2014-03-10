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

#ifndef incl_HPHP_EXT_STREAM_USER_FILTERS_H_
#define incl_HPHP_EXT_STREAM_USER_FILTERS_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class BucketBrigade : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(BucketBrigade);
  CLASSNAME_IS("bucket brigade");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  BucketBrigade() { };
  explicit BucketBrigade(const String& data);

  void prependBucket(CObjRef bucket);
  void appendBucket(CObjRef bucket);
  Object popFront();

  String createString();
private:
  smart::list<Object> m_buckets;
};

///////////////////////////////////////////////////////////////////////////////

class StreamFilter : public ResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(StreamFilter);
  CLASSNAME_IS("stream filter");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  explicit StreamFilter(CObjRef filter): m_filter(filter) { }

  int64_t invokeFilter(Resource in, Resource out, bool closing);
  void invokeOnClose();
private:
  Object m_filter;
};

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(stream_filter_register,
                   const String& filtername,
                   const String& classname);
Array HHVM_FUNCTION(stream_get_filters);
Variant HHVM_FUNCTION(stream_filter_append,
                      CResRef stream,
                      const String& filtername,
                      CVarRef readwrite,
                      CVarRef params);
Variant HHVM_FUNCTION(stream_filter_prepend,
                      CResRef stream,
                      const String& filtername,
                      CVarRef readwrite,
                      CVarRef params);
Variant HHVM_FUNCTION(stream_bucket_make_writeable, CResRef bucket_brigade);
void HHVM_FUNCTION(stream_bucket_append,
                   CResRef bucket_brigade,
                   CObjRef bucket);
void HHVM_FUNCTION(stream_bucket_prepend,
                   CResRef bucket_brigade,
                   CObjRef bucket);

///////////////////////////////////////////////////////////////////////////////
}

#endif //incl_HPHP_EXT_STREAM_USER_FILTERS_H_
