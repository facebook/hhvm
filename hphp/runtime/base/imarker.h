/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_IMARKER_H_
#define incl_HPHP_RUNTIME_IMARKER_H_

#include <list>
#include <bitset>
#include <boost/variant.hpp>
#include "hphp/util/range.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/hphp-raw-ptr.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/indexed-string-map.h"

namespace folly {
template <class T> class Optional;
}

namespace HPHP {
namespace req { template<typename T> struct ptr; }
struct Array;
struct ArrayIter;
struct MArrayIter;
struct String;
struct Variant;
struct ObjectData;
struct ResourceData;
struct StringData;
struct ArrayData;
struct RefData;
struct Func;
struct StringBuffer;
struct Class;
struct TypedValue;
struct NameValueTable;
struct ActRec;
struct Stack;
struct VarEnv;
struct RequestEventHandler;
struct IMarker;
struct AsioContext;
struct Expression;
struct Unit;
struct StreamContext;
struct DateTime;
struct Extension;
template <typename T, bool isLow> struct AtomicSharedPtrImpl;
template <typename T> struct FixedVector;
template <typename T> struct SweepableMember;
template <typename A, typename B, typename P> struct Either;
template<class T, class Allocator> struct TlsPodBag;
template <typename T> struct default_ptr;
template <typename T> struct copy_ptr;
namespace rds { template <typename T> struct Link; }
namespace HHBBC {
struct Type;
struct ClassInfo;
namespace php {
struct Block;
struct Func;
}
}
namespace jit {
struct Block;
struct ALocMeta;
struct Type;
struct TypeConstraint;
}
namespace Compiler { struct Label; }

template <typename F, typename T>
void scan(const T& v, F& mark) {
  mark(v);
}

template <typename F, typename T, template <typename> class C>
void scan(const C<T>& v, F& mark) {
  mark(v);
}

template <typename F, typename Itr>
void scan(Itr start, Itr end, F& mark) {
  while (start != end) scan(*start++, mark);
}

// Interface for marking.
struct IMarker {
  virtual void operator()(const Resource&) = 0;
  virtual void operator()(const Object&) = 0;
  virtual void operator()(const Array&) = 0;
  virtual void operator()(const String&) = 0;
  virtual void operator()(const Variant&) = 0;
  virtual void operator()(const ArrayIter&) = 0;
  virtual void operator()(const MArrayIter&) = 0;
  virtual void operator()(const StringBuffer&) = 0;
  virtual void operator()(const ActRec&) = 0;
  virtual void operator()(const Stack&) = 0;
  virtual void operator()(const VarEnv&) = 0;
  virtual void operator()(const RequestEventHandler&) = 0;
  virtual void operator()(const Extension&) = 0;
  virtual void operator()(const AsioContext&) = 0;

  virtual void operator()(const StringData*) = 0;
  virtual void operator()(const ArrayData*) = 0;
  virtual void operator()(const ObjectData*) = 0;
  virtual void operator()(const ResourceData*) = 0;
  virtual void operator()(const RefData*) = 0;
  virtual void operator()(const Func*) = 0;
  virtual void operator()(const Class*) = 0;
  virtual void operator()(const TypedValue*) = 0;
  virtual void operator()(const NameValueTable*) = 0;
  virtual void operator()(const Unit*) = 0;

  virtual void operator()(const void* start, size_t len) = 0;

  void operator()(const ObjectData& od) { (*this)(&od); }
  void operator()(const ResourceData& rd) { (*this)(&rd); }
  void operator()(const ArrayData& ad) { (*this)(&ad); }
  void operator()(const StringData& sd) { (*this)(&sd); }
  void operator()(const Func& f) { (*this)(&f); }
  void operator()(const RefData& rd) { (*this)(&rd); }
  void operator()(const Class& c) { (*this)(&c); }
  void operator()(const TypedValue& tv) { (*this)(&tv); }
  void operator()(const NameValueTable& nvt) { (*this)(&nvt); }
  void operator()(const Unit& u) { (*this)(&u); }
  void operator()(const Variant* v) { (*this)(*v); }
  void operator()(const Array* a) { (*this)(*a); }
  void operator()(const Resource* r) { (*this)(*r); }
  void operator()(const String* s) { (*this)(*s); }
  void operator()(const RequestEventHandler* reh) { (*this)(*reh); }
  void operator()(const AsioContext* ctx) { (*this)(*ctx); }

  template <typename T>
  void operator()(const typename std::list<T>::iterator& p) {
    scan(*p, *this);
  }

  template <typename T>
  void operator()(const IterRange<T>& r) {
    for (const auto& elem : r) scan(elem, *this);
  }

  template <typename Itr>
  void operator()(Itr start, Itr end) {
    while (start != end) { scan(*start++, *this); }
  }

  template <typename T>
  void operator()(const LowPtr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const LowStringPtr& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const AtomicLowPtr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const std::shared_ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const hphp_raw_ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const req::ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const default_ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const copy_ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T, bool isLow>
  void operator()(const AtomicSharedPtrImpl<T,isLow>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const req::unique_ptr<T>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T, typename D>
  void operator()(const std::unique_ptr<T,D>& p) {
    if (p) scan(*p.get(), *this);
  }

  template <typename T>
  void operator()(const req::list<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const req::vector<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const req::set<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename A, typename B>
  typename std::enable_if<
    (!std::is_fundamental<A>::value && !std::is_fundamental<B>::value),
    void
  >::type operator()(const std::pair<A, B>& p) {
    scan(p.first, *this);
    scan(p.second, *this);
  }

  template <typename A, typename B>
  typename std::enable_if<
    (!std::is_fundamental<A>::value && !std::is_fundamental<B>::value),
    void
  >::type operator()(const std::pair<A, B*>& p) {
    scan(p.first, *this);
    if (p.second) scan(*p.second, *this);
  }

  template <typename A, typename B>
  typename std::enable_if<
    (!std::is_fundamental<A>::value && std::is_fundamental<B>::value),
    void
  >::type operator()(const std::pair<A, B>& p) {
    scan(p.first, *this);
  }

  template <typename A, typename B>
  typename std::enable_if<
    (std::is_fundamental<A>::value && !std::is_fundamental<B>::value),
    void
  >::type operator()(const std::pair<A, B>& p) {
    scan(p.second, *this);
  }

  template <typename A, typename B>
  typename std::enable_if<
    (std::is_fundamental<A>::value && std::is_fundamental<B>::value),
    void
  >::type operator()(const std::pair<A, B>& p) { }

  template <typename K, typename V, typename C, typename A>
  void operator()(const std::map<K, V, C, A>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T, typename C>
  void operator()(const std::set<T, C>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const std::vector<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const std::deque<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const std::deque<T*>& p) {
    for (const auto& elem : p) {
      if (elem) scan(*elem, *this);
    }
  }

  template <typename T>
  void operator()(const std::vector<T*>& p) {
    for (const auto& elem : p) {
      if (elem) scan(*elem, *this);
    }
  }

  template <typename K, typename V>
  void operator()(const boost::container::flat_map<K,V>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename K>
  void operator()(const boost::container::flat_set<K>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename A, typename B, typename P>
  void operator()(const Either<A,B,P>& p) {
    p.match(
      [this](const A& a) { scan(a, *this); },
      [this](const B& b) { scan(b, *this); }
    );
  }

  template <class T, class U, class V, class W>
  void operator()(const std::unordered_map<T,U,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <class T, class U, class V, class W>
  void operator()(const std::unordered_map<T*,U*,V,W>& p) {
    for (const auto& elem : p) {
      if (elem.first) scan(*elem.first, *this);
      if (elem.second) scan(*elem.second, *this);
    }
  }

  template <class T, class V, class W>
  void operator()(const std::unordered_set<T,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <class T, class V, class W>
  void operator()(const std::unordered_set<T*,V,W>& p) {
    for (const auto& elem : p) if (elem) scan(*elem, *this);
  }

  template <class T, class U, class V, class W>
  void operator()(const req::hash_map<T,U,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <class T, class U, class V, class W>
  void operator()(const jit::hash_map<T,U,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template<class T,
           bool CaseSensitive,
           class Index,
           Index InvalidIndex = Index(-1)>
  void operator()(
    const IndexedStringMap<T,CaseSensitive,Index,InvalidIndex>& p
  ) {
    for (const auto& elem : p.range()) scan(elem, *this);
  }

  template <class T, class U, class V, class W>
  void operator()(const hphp_hash_map<T,U,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <class T, class U, class V, class W>
  void operator()(const hphp_hash_map<T,U*,V,W>& p) {
    for (const auto& elem : p) {
      scan(elem.first, *this);
      if (elem.second) scan(*elem.second, *this);
    }
  }

  template <class T, class U, class V, class W>
  void operator()(const hphp_hash_map<T*,U*,V,W>& p) {
    for (const auto& elem : p) {
      if (elem.first) scan(*elem.first, *this);
      if (elem.second) scan(*elem.second, *this);
    }
  }

  template <class T, class U, class V>
  void operator()(const hphp_hash_set<T,U,V>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <class T, class V, class W>
  void operator()(const req::hash_set<T,V,W>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T, size_t N>
  void operator()(const std::array<T,N>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T>
  void operator()(const FixedVector<T>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template <typename T, size_t N>
  void operator()(const TinyVector<T,N>& p) {
    for (const auto& elem : p) scan(elem, *this);
  }

  template<class T, class A>
  void operator()(const TlsPodBag<T,A>& p) {
    const_cast<TlsPodBag<T,A>&>(p).for_each(
      [this](const T& v) { scan(v, *this); });
  }

  template <typename ...T>
  void operator()(const boost::variant<T...>& p) {
    p.apply_visitor(variant_visitor(*this));
  }

  template <class T>
  void operator()(const folly::Optional<T>& p) {
    if (p) scan(*p, *this);
  }

  template <typename T>
  void operator()(const rds::Link<T>& p) {
    scan(*p, *this);
  }

  template <typename T>
  void operator()(const SweepableMember<T>& p) {
    // TODO (t6956600): do something clever here.
  }

  // Always ignore std::enable_shared_from_this.
  template <typename T>
  void operator()(const std::enable_shared_from_this<T>&) { }

  // Ignored other halves of a std::pairs.
  void operator()(const Compiler::Label*) { }
  void operator()(const HHBBC::php::Block*) { }
  void operator()(const Compiler::Label&) { }
  void operator()(const HHBBC::php::Block&) { }
  void operator()(const HHBBC::php::Func&) { }
  void operator()(const HHBBC::Type&) { }
  void operator()(const HHBBC::ClassInfo&) { }
  void operator()(const Expression&) { }
  void operator()(const std::string&) { }
  void operator()(const jit::Block*) { }
  void operator()(const jit::ALocMeta&) { }
  void operator()(const jit::TypeConstraint&) { }
  template <size_t N> void operator()(const std::bitset<N>&) { }

protected:
  struct variant_visitor : boost::static_visitor<> {
    template <typename T> void operator()(const T& v) { m_mark(v); }
    explicit variant_visitor(IMarker& mark) : m_mark(mark) { }
    IMarker& m_mark;
  };

  ~IMarker() {}
};

// This class is a temporary bridge that allows ResourceData scan functions to
// work whether or not generated scan functions are present in a sandbox.
struct scanner {
  template <typename F, typename T> void scan(const T& v, F& mark) {
    HPHP::scan(v, mark);
  }
  template <typename F> void scan(const ResourceData& v, F& mark) {
    HPHP::scan(v, mark);
  }
  template <typename F> void scan(const AsioContext& v, F& mark) {
    // do nothing for now.
  }
};

// bridge between the templated-based marker interface and the
// virtual-call based marker interface.
template<class F> struct ExtMarker final: IMarker {
  explicit ExtMarker(F& mark) : mark_(mark) {}
  void operator()(const Array& p) override { mark_(p); }
  void operator()(const Object& p) override { mark_(p); }
  void operator()(const Resource& p) override { mark_(p); }
  void operator()(const String& p) override { mark_(p); }
  void operator()(const Variant& p) override { mark_(p); }
  void operator()(const ArrayIter& p) override { mark_(p); }
  void operator()(const MArrayIter& p) override { mark_(p); }
  void operator()(const StringBuffer& p) override { mark_(p); }
  void operator()(const ActRec& p) override { mark_(p); }
  void operator()(const Stack& p) override { mark_(p); }
  void operator()(const VarEnv& p) override { mark_(p); }
  void operator()(const RequestEventHandler& p) override { mark_(p); }
  void operator()(const Extension& p) override { mark_(p); }
  void operator()(const AsioContext& p) override { mark_(p); }

  void operator()(const StringData* p) override { mark_(p); }
  void operator()(const ArrayData* p) override { mark_(p); }
  void operator()(const ObjectData* p) override { mark_(p); }
  void operator()(const ResourceData* p) override { mark_(p); }
  void operator()(const RefData* p) override { mark_(p); }
  void operator()(const Func* p) override { mark_(p); }
  void operator()(const Class* p) override { mark_(p); }
  void operator()(const TypedValue* p) override { mark_(*p); }
  void operator()(const NameValueTable* p) override { mark_(*p); }
  void operator()(const Unit* p) override { mark_(p); }

  void operator()(const void* start, size_t len) override {
    mark_(start, len);
  }
private:
  F& mark_;
};

}
#endif
