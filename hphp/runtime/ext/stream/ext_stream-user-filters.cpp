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

#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_array.h"
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

class StreamUserFilters : public RequestEventHandler {
 public:
  virtual ~StreamUserFilters() {}
  Array m_registeredFilters;

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

  virtual void requestInit() {
    vm_call_user_func(s_default_filters_register_func, empty_array);
  }

  virtual void requestShutdown() {}
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

    auto file = stream.getTyped<File>();
    assert(file);

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
    Resource ret;
    if (mode & k_STREAM_FILTER_READ) {
      auto resource = createInstance(func_name,
                                     stream,
                                     filtername,
                                     params);
      if (resource.isNull()) {
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
                                     stream,
                                     filtername,
                                     params);
      if (resource.isNull()) {
        return false;
      }
      ret = resource;
      if (append) {
        file->appendWriteFilter(resource);
      } else {
        file->prependWriteFilter(resource);
      }
    }
    return ret;
  }

  Resource createInstance(const char* php_func,
                          const Resource& stream,
                          const String& filter,
                          const Variant& params) {
    auto class_name = m_registeredFilters.rvalAt(filter).asCStrRef();
    Class* class_ = Unit::lookupClass(class_name.get());
    Object obj = null_object;

    if (LIKELY(class_ != nullptr)) {
      ArrayInit ctor_args(3);
      ctor_args.set(stream);
      ctor_args.set(filter);
      ctor_args.set(params);
      obj = g_context->createObject(class_name.get(), ctor_args.toArray());
      auto created = obj->o_invoke(s_onCreate, Array::Create());
      /* - true: documented value for success
       * - null: undocumented default successful value
       * - false: documented value for failure
       */
      if (!(created.isNull() || created.toBoolean())) {
        obj = null_object;
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
      return null_resource;
    }

    return Resource(NEWOBJ(StreamFilter)(obj, stream));
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(StreamUserFilters, s_stream_user_filters);

///////////////////////////////////////////////////////////////////////////////
// StreamFilter

int64_t StreamFilter::invokeFilter(Resource in,
                                   Resource out,
                                   bool closing) {
  auto consumedTV = make_tv<KindOfInt64>(0);
  auto consumedRef = RefData::Make(consumedTV);

  ArrayInit params(4);
  params.set(in);
  params.set(out);
  params.set(consumedRef);
  params.set(closing);
  return m_filter->o_invoke(s_filter, params.toArray()).toInt64();
}

void StreamFilter::invokeOnClose() {
  m_filter->o_invoke(s_onClose, Array::Create());
}

bool StreamFilter::remove() {
  if (m_stream.isNull()) {
    return false;
  }
  auto file = m_stream.getTyped<File>();
  assert(file);
  Resource rthis(this);
  auto ret = file->removeFilter(rthis);
  m_stream = null_resource;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// BucketBrigade

BucketBrigade::BucketBrigade(const String& data) {
  ArrayInit ai(2);
  ai.set(data);
  ai.set(data.length());
  auto bucket = g_context->createObject(s_bucket_class.get(), ai.toArray());
  appendBucket(bucket);
}

void BucketBrigade::appendBucket(const Object& bucket) {
  m_buckets.push_back(bucket);
}

void BucketBrigade::prependBucket(const Object& bucket) {
  m_buckets.push_front(bucket);
}

Object BucketBrigade::popFront() {
  if (m_buckets.empty()) {
    return null_object;
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
    return Array::Create();
  }
  return f_array_keys(filters).toArray();
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
  auto filter = resource.getTyped<StreamFilter>();
  assert(filter);
  return filter->remove();
}

Variant HHVM_FUNCTION(stream_bucket_make_writeable, const Resource& bb_res) {
  auto brigade = bb_res.getTyped<BucketBrigade>();
  assert(brigade);
  auto ret = brigade->popFront();
  return ret;
}

void HHVM_FUNCTION(stream_bucket_append, const Resource& bb_res, const Object& bucket) {
  auto brigade = bb_res.getTyped<BucketBrigade>();
  assert(brigade);
  brigade->appendBucket(bucket);
}

void HHVM_FUNCTION(stream_bucket_prepend, const Resource& bb_res, const Object& bucket) {
  auto brigade = bb_res.getTyped<BucketBrigade>();
  assert(brigade);
  brigade->prependBucket(bucket);
}

///////////////////////////////////////////////////////////////////////////////

}
