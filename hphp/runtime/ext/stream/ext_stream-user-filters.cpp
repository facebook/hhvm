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

#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/array/ext_array.h"
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/system/constants.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const StaticString s_filter("filter");
const StaticString s_onCreate("onCreate");
const StaticString s_onClose("onClose");

const StaticString s_bucket_class("__SystemLib\\StreamFilterBucket");

///////////////////////////////////////////////////////////////////////////////

const StaticString s_default_filters_register_func(
  "__SystemLib\\register_default_stream_filters");

///////////////////////////////////////////////////////////////////////////////

struct StreamFilterRepository {
  void add(const String& key, const Variant& v) {
    m_filters.add(key, v);
  }

  void detach() {
    m_filters.detach();
  }

  bool exists(const String& needle) const {
    if (m_filters.exists(needle.toKey())) {
      return true;
    }
    /* Could not find exact match, now try wildcard match */
    int lastDotPos = needle.rfind('.');
    if (String::npos == lastDotPos) {
      return false;
    }
    String wildcard = needle.substr(0, lastDotPos) + ".*";
    return m_filters.exists(wildcard.toKey());
  }

  Variant rvalAt(const String& needle) const {
    /* First try to find exact match, afterwards try wildcard matches */
    int lastDotPos = needle.rfind('.');
    if (String::npos == lastDotPos || m_filters.exists(needle.toKey())) {
      return m_filters.rvalAtRef(needle);
    }
    String wildcard = needle.substr(0, lastDotPos) + ".*";
    return m_filters.rvalAtRef(wildcard);
  }

  const Array& filtersAsArray() const {
    return m_filters;
  }

  bool isNull() const {
    return m_filters.isNull();
  }

private:
  Array m_filters;
};

///////////////////////////////////////////////////////////////////////////////

struct StreamUserFilters final : RequestEventHandler {
  virtual ~StreamUserFilters() {}

  bool registerFilter(const String& name, const String& class_name) {
    if (m_registeredFilters.exists(name)) {
      return false;
    }
    m_registeredFilters.add(name, class_name);
    return true;
  }

  Variant prepend(const Resource& stream,
                 const String& filtername,
                 const Variant& readwrite,
                 const Variant& params) {
    return appendOrPrependFilter(stream,
                                 filtername,
                                 readwrite,
                                 params,
                                 /* append = */ false);
  }

  Variant append(const Resource& stream,
                 const String& filtername,
                 const Variant& readwrite,
                 const Variant& params) {
    return appendOrPrependFilter(stream,
                                 filtername,
                                 readwrite,
                                 params,
                                 /* append = */ true);
  }

  void requestInit() override {
    vm_call_user_func(s_default_filters_register_func, empty_array_ref);
  }

  void requestShutdown() override {
    m_registeredFilters.detach();
  }

  void vscan(IMarker& mark) const override {
    mark(m_registeredFilters.filtersAsArray());
  }

private:
  Variant appendOrPrependFilter(const Resource& stream,
                 const String& filtername,
                 const Variant& readwrite,
                 const Variant& params,
                 bool append) {
    const char* func_name =
      append ? "stream_filter_append()" : "stream_filter_prepend()";

    if (!m_registeredFilters.exists(filtername)) {
      raise_warning("%s: unable to locate filter \"%s\"",
                    func_name,
                    filtername.data());
      return false;
    }

    auto file = cast<File>(stream);
    int mode = readwrite.toInt32();
    if (!mode) {
      auto str = file->getMode();
      /* The documentation says a read filter is only created for 'r' and '+'
       * modes, but the implementation will always create one unless
       * STREAM_FILTER_WRITE is passed.
       *
       * This branch is only executed if no STREAM_FILTER* args were passed,
       * so we always create a READ filter.
       */
      mode = k_STREAM_FILTER_READ;
      if (str.find('+') != -1 || str.find('w') != -1 || str.find('a') != -1) {
        mode |= k_STREAM_FILTER_WRITE;
      }
    }
    if (!(mode & k_STREAM_FILTER_ALL)) {
      return false;
    }

    // If it's ALL we create two resources, but only return one - this
    // matches Zend, and is the documented behavior.
    req::ptr<StreamFilter> ret;
    if (mode & k_STREAM_FILTER_READ) {
      auto resource = createInstance(func_name,
                                     file,
                                     filtername,
                                     params);
      if (!resource) {
        return false;
      }
      ret = resource;
      if (append) {
        file->appendReadFilter(resource);
      } else {
        file->prependReadFilter(resource);
      }
    }
    if (mode & k_STREAM_FILTER_WRITE) {
      auto resource = createInstance(func_name,
                                     file,
                                     filtername,
                                     params);
      if (!resource) {
        return false;
      }
      ret = resource;
      if (append) {
        file->appendWriteFilter(resource);
      } else {
        file->prependWriteFilter(resource);
      }
    }
    return Variant(std::move(ret));
  }

  req::ptr<StreamFilter> createInstance(const char* php_func,
                                        req::ptr<File> stream,
                                        const String& filter,
                                        const Variant& params) {
    auto class_name = m_registeredFilters.rvalAt(filter).asCStrRef();
    Class* class_ = Unit::getClass(class_name.get(), true);
    Object obj = Object();

    if (LIKELY(class_ != nullptr)) {
      PackedArrayInit ctor_args(3);
      ctor_args.append(Variant(stream));
      ctor_args.append(filter);
      ctor_args.append(params);
      obj = Object::attach(
        g_context->createObject(class_name.get(), ctor_args.toArray())
      );
      auto created = obj->o_invoke(s_onCreate, Array::Create());
      /* - true: documented value for success
       * - null: undocumented default successful value
       * - false: documented value for failure
       */
      if (!(created.isNull() || created.toBoolean())) {
        obj.reset();
      }
    } else {
      raise_warning("%s: user-filter \"%s\" requires class \"%s\", but that "
                    "class " "is not defined",
                    php_func,
                    filter.data(),
                    class_name.data());
      // Fall through, as to match Zend, the warning below should also be raised
    }

    if (obj.isNull()) {
      raise_warning("%s: unable to create or locate filter \"%s\"",
                    php_func,
                    filter.data());
      return nullptr;
    }

    return req::make<StreamFilter>(obj, stream);
  }

public:
  StreamFilterRepository m_registeredFilters;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(StreamUserFilters, s_stream_user_filters);

///////////////////////////////////////////////////////////////////////////////
// StreamFilter

int64_t StreamFilter::invokeFilter(const req::ptr<BucketBrigade>& in,
                                   const req::ptr<BucketBrigade>& out,
                                   bool closing) {
  auto consumedTV = make_tv<KindOfInt64>(0);
  auto consumedRef = RefData::Make(consumedTV);

  PackedArrayInit params(4);
  params.append(Variant(in));
  params.append(Variant(out));
  params.append(consumedRef);
  params.append(closing);
  return m_filter->o_invoke(s_filter, params.toArray()).toInt64();
}

void StreamFilter::invokeOnClose() {
  m_filter->o_invoke(s_onClose, Array::Create());
}

bool StreamFilter::remove() {
  if (!m_stream) {
    return false;
  }
  auto ret = m_stream->removeFilter(req::ptr<StreamFilter>(this));
  m_stream.reset();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// BucketBrigade

BucketBrigade::BucketBrigade(const String& data) {
  PackedArrayInit ai(2);
  ai.append(data);
  ai.append(data.length());
  auto bucket = g_context->createObject(s_bucket_class.get(), ai.toArray());
  appendBucket(Object::attach(bucket));
}

void BucketBrigade::appendBucket(const Object& bucket) {
  m_buckets.push_back(bucket);
}

void BucketBrigade::prependBucket(const Object& bucket) {
  m_buckets.push_front(bucket);
}

Object BucketBrigade::popFront() {
  if (m_buckets.empty()) {
    return Object();
  }
  auto bucket = m_buckets.front();
  m_buckets.pop_front();
  return bucket;
}

String BucketBrigade::createString() {
  StringBuffer buffer;
  for (auto& bucket_obj: m_buckets) {
    buffer.append(bucket_obj.toString());
  }
  return buffer.detach();
}

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(stream_filter_register,
                   const String& name,
                   const String& classname) {
  return s_stream_user_filters.get()->registerFilter(name, classname);
}

Array HHVM_FUNCTION(stream_get_filters) {
  auto filters = s_stream_user_filters.get()->m_registeredFilters;
  if (UNLIKELY(filters.isNull())) {
    return empty_array();
  }
  return array_keys_helper(filters.filtersAsArray()).toArray();
}

Variant HHVM_FUNCTION(stream_filter_append,
                      const Resource& stream,
                      const String& filtername,
                      const Variant& readwrite,
                      const Variant& params) {
  return s_stream_user_filters.get()->append(stream,
                                             filtername,
                                             readwrite,
                                             params);
}

Variant HHVM_FUNCTION(stream_filter_prepend,
                      const Resource& stream,
                      const String& filtername,
                      const Variant& readwrite,
                      const Variant& params) {
  return s_stream_user_filters.get()->prepend(stream,
                                              filtername,
                                              readwrite,
                                              params);
}

bool HHVM_FUNCTION(stream_filter_remove, const Resource& resource) {
  return cast<StreamFilter>(resource)->remove();
}

Variant HHVM_FUNCTION(stream_bucket_make_writeable, const Resource& bb_res) {
  return cast<BucketBrigade>(bb_res)->popFront();
}

void HHVM_FUNCTION(stream_bucket_append, const Resource& bb_res, const Object& bucket) {
  cast<BucketBrigade>(bb_res)->appendBucket(bucket);
}

void HHVM_FUNCTION(stream_bucket_prepend, const Resource& bb_res, const Object& bucket) {
  cast<BucketBrigade>(bb_res)->prependBucket(bucket);
}

const StaticString
  s_STREAM_FILTER_READ("STREAM_FILTER_READ"),
  s_STREAM_FILTER_WRITE("STREAM_FILTER_WRITE"),
  s_STREAM_FILTER_ALL("STREAM_FILTER_ALL");

void StandardExtension::initStreamUserFilters() {
#define SFCNS(v) Native::registerConstant<KindOfInt64> \
                         (s_STREAM_FILTER_##v.get(), k_STREAM_FILTER_##v)
  SFCNS(READ);
  SFCNS(WRITE);
  SFCNS(ALL);
#undef SFCNS

  HHVM_FE(stream_get_filters);
  HHVM_FE(stream_filter_register);
  HHVM_FE(stream_filter_append);
  HHVM_FE(stream_filter_prepend);
  HHVM_FE(stream_filter_remove);
  HHVM_FE(stream_bucket_make_writeable);
  HHVM_FE(stream_bucket_append);
  HHVM_FE(stream_bucket_prepend);

  loadSystemlib("stream-user-filters");
}

///////////////////////////////////////////////////////////////////////////////
}
