#ifndef HPHP_THRIFT_RELATIVEPTR_H
#define HPHP_THRIFT_RELATIVEPTR_H

#include <boost/noncopyable.hpp>

namespace apache { namespace thrift {

typedef uint8_t byte;

// Relative Ptr - The key to relocatable object graphs
// TODO: expose 'OffsetType' as a type parameter in freeze()
template<class T,
         class OffsetType = int32_t>
class RelativePtr : private boost::noncopyable {
  OffsetType offset_;
 public:
  RelativePtr() {
    reset(nullptr);
  }

  explicit RelativePtr(T* ptr) {
    reset(ptr);
  }

  void reset(T* ptr = nullptr) {
    if (!ptr) {
      offset_ = 0;
      return;
    }
    const byte* target = reinterpret_cast<const byte*>(ptr);
    const byte* origin = reinterpret_cast<const byte*>(this);
    offset_ = target - origin;
  }

  T* get() const {
    if (!offset_) {
      return nullptr;
    }
    const byte* origin =
      reinterpret_cast<const byte*>(this);
    const byte* target =
      reinterpret_cast<const byte*>(origin + offset_);
    return reinterpret_cast<T*>(target);
  }

  T& operator*() const {
    return *get();
  }
};

}} //apache::thrfit

#endif//THRIFT_RELATIVEPTR_H_
