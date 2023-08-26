/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/super-inlining-bros.h"

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke-iter.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/req-tiny-vector.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/method-lookup.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <sstream>

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

namespace {

TRACE_SET_MOD(sib);

const StaticString s_MysteryBox("MysteryBox");
const StaticString s_stdin("STDIN");
const StaticString s_stdout("STDOUT");
const StaticString s_stderr("STDERR");


bool heapObjIsStatic(HeapObject* obj) {
  return haveCount(obj->kind()) &&
         static_cast<MaybeCountable*>(obj)->isStatic();
}

bool heapObjIsUncounted(HeapObject* obj) {
  return haveCount(obj->kind()) &&
         static_cast<MaybeCountable*>(obj)->isUncounted();
}

HeapObject* findHeapObjStart(HeapObject* ptr) {
  if (ptr->kind() == HeaderKind::Object) {
    // Find memo slots when enqueueing an object.
    auto const obj = reinterpret_cast<ObjectData*>(ptr);
    auto const objOff = obj->getVMClass()->memoSize();

    ptr = reinterpret_cast<HeapObject*>(
      reinterpret_cast<uintptr_t>(ptr) - objOff);
    assertx(objOff == 0 ||
            reinterpret_cast<MemoNode*>(ptr)->objOff() == objOff);
    return ptr;
  }
  return ptr;
}
//////////////////////////////////////////////////////////////////////////////
// Scanner is a heap object walking utility that calls lambdas with references
// to areas in the heap that are relevant for SIB manipulaitons.
// For now we have ProcessTV and ProcessHeapPtr lambdas:
//   - ProcessHeapPtr receives a reference to all HeapObjects*
//   - ProcessTV receives a tv_lval referencing all TVs
// For tracking heap object starts we also have the StartHeapObject lambda.
// StartHeapObject and ProcessTV can return an object that will be held as
// an RAII guard until done with the heap object or tv processing.
//
// In combination with Tracer (which just visits heap obects) we can walk the
// heap and perform maniputations or inspection as we go.
//
// Rough usage:
// Scanner strScanner([&](tv_lval tv) {
//   if (isResourceType(tv.type())) {
//     auto const& data = mysteries[tv.val().num];
//     print(folly::sformat("MysteryBox: {}", show(data.provenance)));
//   }
//   print(std::string("Type: ") + tname(tv.type()));
//   return Indent(indent);
// }, [&](HeapObject** obj) {
//   printHeapObj(*obj);
// }, [&](HeapObject* obj) {
//   printHeapObj(obj);
//   return Indent(indent);
// });
//
// Tracer tracer([&](HeapObject* obj) {
//   strScanner.scan(obj);
// });
// for (auto root : roots) {
//   strScanner.scan(&root);
//   tracer.enqueue(&root);
// }
// tracer.trace();

template<
  typename ScanTV,
  typename ScanHeapObj,
  typename ScanHeapObjPtr
>
struct Scanner {
  Scanner(
      ScanTV&& scanTV,
      ScanHeapObjPtr&& scanHeapObjPtr,
      ScanHeapObj&& scanHeapObj
  ) : scanTV(scanTV)
    , scanHeapObj(scanHeapObj)
    , scanHeapObjPtr(scanHeapObjPtr)
  {}

  struct ScanException : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  void scan(tv_lval tv) {
    UNUSED auto const guard = [&]() {
      if constexpr (std::is_same_v<decltype(scanTV(tv)), void>) {
        scanTV(tv);
        return nullptr;
      } else {
        return scanTV(tv);
      }
    }();
    if (isRefcountedType(dt_modulo_persistence(tv.type())) &&
        !isResourceType(tv.type())) {
      scan(reinterpret_cast<HeapObject**>(&tv.val().pcnt));
    }
  }

  void scan(HeapObject* obj) {
    if (heapObjIsStatic(obj)) return;
    UNUSED auto const guard = [&]() {
      if constexpr (std::is_same_v<decltype(scanHeapObj(obj)), void>) {
        scanHeapObj(obj);
        return nullptr;
      } else {
        return scanHeapObj(obj);
      }
    }();

    auto const headerName = header_names[int(obj->kind())];
    switch (obj->kind()) {
      // TODO: All these scanners have to be audited for more relevant
      // data.  Eg. the class* in an object
      case HeaderKind::Vec:
        return scanVec(static_cast<ArrayData*>(obj));
      case HeaderKind::Dict:
        return scanDict(static_cast<VanillaDict*>(obj));
      case HeaderKind::Keyset:
        return scanKeyset(static_cast<VanillaKeyset*>(obj));
      case HeaderKind::MemoData:
        // We assert the memo slots are unused.  So just skip the memo data.
        obj = memoObj(obj);
      case HeaderKind::Object:
        // NativeObject should hit the NativeData case below.
        return scanObject(static_cast<ObjectData*>(obj));
      case HeaderKind::String:
        // Nothing to see.
        return;
      case HeaderKind::WaitHandle: {
        auto const wh = static_cast<c_Awaitable*>(obj);
        assertx(wh->getKind() == c_Awaitable::Kind::Static);
        if (!wh->isSucceeded()) {
          throw ScanException(folly::sformat(
              "Encountered failed static wait handle"));
        }
        return scan(wh->getResultAsLval());
      }
      case HeaderKind::BespokeVec:
      case HeaderKind::BespokeDict:
      case HeaderKind::BespokeKeyset: {
        auto ba = static_cast<BespokeArray*>(obj);
        auto const layout = jit::ArrayLayout::FromArray(ba);
        if (layout.is_struct()) {
          return scanStructDict(ba);
        }
        throw ScanException(folly::sformat(
            "Unhandled type bespoke scanned ({}, {})", headerName,
            layout.describe()));
      }
      default:
        always_assert_flog(
          false, "Hit unhandled heap object kind {}", headerName);
    }
  }

private:
  void scan(HeapObject** dptr) {
    auto ptr = *dptr;
    auto const headerName = header_names[int(ptr->kind())];
    switch (ptr->kind()) {
      case HeaderKind::Vec:
      case HeaderKind::Dict:
      case HeaderKind::Keyset:
      case HeaderKind::MemoData:
      case HeaderKind::String:
      case HeaderKind::WaitHandle:
      case HeaderKind::BespokeVec:
      case HeaderKind::BespokeDict:
      case HeaderKind::BespokeKeyset:
      case HeaderKind::Object:
        break;
      case HeaderKind::Closure:
      case HeaderKind::ClosureHdr:
      case HeaderKind::ClsMeth:
      case HeaderKind::RClsMeth:
      case HeaderKind::RFunc:
        // TODO: Handle above trash.
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::AwaitAllWH:
      case HeaderKind::ConcurrentWH:
      case HeaderKind::NativeData:
      case HeaderKind::AsyncFuncFrame:
      case HeaderKind::Cpp:
      case HeaderKind::SmallMalloc:
      case HeaderKind::BigMalloc:
      case HeaderKind::Resource:
      case HeaderKind::NativeObject:
      case HeaderKind::Vector:
      case HeaderKind::Map:
      case HeaderKind::Set:
      case HeaderKind::Pair:
      case HeaderKind::ImmVector:
      case HeaderKind::ImmMap:
      case HeaderKind::ImmSet:
        throw ScanException(folly::sformat(
            "Unhandled type being scanned ({})", headerName));
      case HeaderKind::Free:
      case HeaderKind::Hole:
      case HeaderKind::Slab:
        always_assert_flog(false,
                           "Hit unhandled heap object kind during enqueue {}",
                           headerName);
    }
    scanHeapObjPtr(dptr);
  }

  void scanVec(ArrayData* a) {
    for (uint32_t i = 0; i < a->size(); ++i) {
      const auto lval = VanillaVec::LvalUncheckedInt(a, i);
      scan(lval);
    }
  }

  void scanDict(VanillaDict* a) {
    auto elm = a->data();
    for (auto i = VanillaDict::IterEnd(a); i--; elm++) {
      if (!elm->isTombstone()) {
        if (elm->hasStrKey()) {
          scan(reinterpret_cast<HeapObject**>(&elm->skey));
        }
        scan(&(elm->data));
      }
    }
  }

  void scanKeyset(VanillaKeyset* a) {
    auto elm = a->data();
    for (auto i = VanillaKeyset::IterEnd(a); i--; elm++) {
      if (!elm->isTombstone()) {
        scan(&(elm->tv));
      }
    }
  }

  void scanObject(ObjectData* obj) {
    using Attr = ObjectData::Attribute;
    std::string error{};
    auto const check = [&] (Attr attr, std::string msg) {
      if (obj->getAttribute(attr)) {
        error += error.size() ? "|" + msg : msg;
      }
    };
    check(Attr::IsWeakRefed, "IsWeakRefed");
    check(Attr::HasDynPropArr, "HasDynPropArr");
    check(Attr::UsedMemoCache, "UsedMemoCache");
    if (error.size()) {
      throw ScanException(folly::sformat(
          "Unhandled object due to attribute(s) ({})", error));
    }
    if (obj->getVMClass()->needInitialization()) {
      throw ScanException(folly::sformat(
          "Unhandled object due to class needing initialization ({})",
          obj->getVMClass()->name()));
    }
    for (auto slot = obj->getVMClass()->declPropInit().size(); slot-- > 0;) {
      // TODO: Handle modification of slot ordering during serde.
      // handle class seriailization as well.
      scan(obj->propLvalAtOffset(slot));
    }
  }

  void scanStructDict(BespokeArray* ba) {
    bespoke::StructDictIterateLvals(ba, [&](tv_lval tv) {
      scan(tv);
    });
  }

  ScanTV scanTV;
  ScanHeapObj scanHeapObj;
  ScanHeapObjPtr scanHeapObjPtr;
};

template<typename HeapObjVisitor>
struct Tracer {
  using RetType = std::result_of_t<HeapObjVisitor(HeapObject*)>;
  static constexpr bool NoPOVisit = std::is_same_v<RetType, void>;
  using POVisit = std::conditional_t<NoPOVisit, int, RetType>;

  explicit Tracer(HeapObjVisitor&& heapObjVisitor)
    : heapObjVisitor(heapObjVisitor)
  {}

  template<typename T>
  void enqueue(T toEnqueue) {
    Scanner scanner([](tv_lval) {}, [this] (HeapObject** obj) {
      scanHeapObjPtr(obj);
    }, [](HeapObject*){});
    scanner.scan(toEnqueue);
  }

  void trace() {
    while (!m_work.empty()) {
      auto& w = m_work.back();
      if (!w.visitor) {
        if constexpr (NoPOVisit) {
          heapObjVisitor(w.ptr);
          m_work.pop_back();
        } else {
          w.visitor.emplace(heapObjVisitor(w.ptr));
        }
        enqueue(w.ptr);
      } else {
        m_work.pop_back();
      }
    }
  }
private:
  void scanHeapObjPtr(HeapObject** obj) {
    if (heapObjIsStatic(*obj)) return;
    auto const ptr = findHeapObjStart(*obj);
    auto const [_, inserted] = m_visited.insert(ptr);
    if (inserted) {
      m_work.emplace_back(Work{ptr, std::nullopt});
    }
  }

  HeapObjVisitor heapObjVisitor;
  struct Work {
    HeapObject* ptr;
    Optional<POVisit> visitor;
  };
  std::vector<Work> m_work;
  jit::fast_set<HeapObject*> m_visited;
};

template<typename HeapObjVisitor>
struct POTracer {
  POTracer(HeapObjVisitor&& heapObjVisitor, HeapObject* obj)
    : heapObjVisitor(heapObjVisitor)
    , obj(obj)
  {}
  POTracer(const POTracer&) = delete;
  POTracer& operator=(const POTracer&) = delete;
  POTracer(POTracer&& o) noexcept
    : heapObjVisitor(std::move(o.heapObjVisitor))
    , obj(o.obj)
  {
    o.obj = nullptr;
  }
  POTracer& operator=(POTracer&& o) noexcept {
    heapObjVisitor = std::move(o.heapObjVisitor);
    obj = o.obj;
    o.obj = nullptr;
    return *this;
  }
  ~POTracer() {
    if (obj) {
      heapObjVisitor(obj);
    }
  }
  HeapObjVisitor heapObjVisitor;
  HeapObject* obj;
};

}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_RESOURCE_ALLOCATION(MysteryBox);

struct MysteryBoxData {
  MysteryBoxProvenance provenance;
  jit::Type type;

  std::vector<MysteryBoxConstraint> constraints;
};

namespace {

MysteryBoxData* unbox(TypedValue tv) {
  if (!tvIsResource(tv)) return nullptr;
  auto const data = tv.val().pres->data();
  if (!data->instanceof<MysteryBox>()) return nullptr;
  auto const box = dynamic_cast<const MysteryBox*>(data);
  assertx(box && box->data);
  return box->data;
}

std::string show(const MysteryBoxProvenance& p) {
  switch (p.type) {
    case MysteryBoxType::Unknown: return "Unknown";
    case MysteryBoxType::Context: return "Context";
    case MysteryBoxType::Local:   return folly::sformat("Local:{}", p.local);
  }
  always_assert(false);
}

std::string show(TypedValue tv) {
  auto const box = unbox(tv);
  if (box) {
    auto const type = box->type.toString();
    return folly::sformat("MysteryBox({}, {})", show(box->provenance), type);
  }
  auto const data = tv.val();
  auto const type = dt_modulo_persistence(tv.type());
  if (tvIsNull(tv)) return tname(type);
  if (tvIsDouble(tv)) return folly::sformat("Double({})", data.dbl);
  if (tvIsInt(tv) || tvIsBool(tv)) return folly::sformat("Int({})", data.num);
  if (tvIsObject(tv)) {
    auto const cls = data.pobj->getVMClass();
    return folly::sformat("Object(\"{}\")", cls->name()->data());
  }
  if (tvIsString(tv)) {
    auto const str = folly::cEscape<std::string>(data.pstr->data());
    return folly::sformat("String(\"{}\")", str);
  }
  return folly::sformat("{}({})", tname(type), data.pcnt);
}

}

req::ptr<MysteryBox> MysteryBox::Make() {
  return Make(jit::TCell);
}

req::ptr<MysteryBox> MysteryBox::Make(const jit::Type& type) {
  auto const provenance = MysteryBoxProvenance { MysteryBoxType::Unknown };
  return req::make<MysteryBox>(new MysteryBoxData({provenance, type, {}}));
}

req::ptr<MysteryBox> MysteryBox::Make(folly::StringPiece input) {
  auto type = RepoAuthType{};
  ParseRepoAuthType(input, type);
  return Make(jit::typeFromRAT(type, nullptr));
}

bool MysteryBox::IsMysteryBox(TypedValue tv) {
  return unbox(tv);
}

bool MysteryBox::TryConstrain(tv_lval lval, const TypeConstraint& constraint,
                              const Class* ctx, const Class* propDecl) {
  if (!g_context->doingInlineInterp()) return false;
  auto const box = unbox(*lval);
  if (!box) return false;
  // TODO(kshaunak): Track a jit:Type and skip constraints that are already
  // known to be satisfied. Also, create a new MysteryBox in case of coercion.
  FTRACE(2, "    Tracking type constraint to {}: {}\n",
         show(*lval), constraint.displayName());
  box->constraints.push_back({
    constraint,
    ctx,
    propDecl
  });
  return true;
}

MysteryBox::MysteryBox(MysteryBoxData* data, bool unowned)
  : data(data), unowned(unowned) {}

MysteryBox::~MysteryBox() {
  if (unowned) return;
  delete data;
}

const String& MysteryBox::o_getClassNameHook() const {
  return s_MysteryBox;
}

//////////////////////////////////////////////////////////////////////////////
namespace {

struct RenderOnce : ROMRenderer {
  RenderOnce(TypedValue ctx, uint32_t nargs, TypedValue* args, bool partial)
    : ctx(ctx), nargs(nargs), args(args), partial(partial) {}
  void checkConstraint(const MysteryBoxProvenance& provenance,
                       const MysteryBoxConstraint& constraint) override {
    if (partial) return;
    auto& tv = selectMysteryBox(provenance);
    auto const ctx = constraint.ctx;
    auto const& tc = constraint.tc;
    auto const okay = constraint.propDecl ? tc.checkProp(&tv, ctx)
                                          : tc.check(&tv, ctx);
    if (!okay) {
      // TODO: Handle coercions, which require dependent mystery boxes.
      throw RenderException(folly::sformat(
          "Failed to constrain mystery local {}, {} fails constraint {}",
          show(provenance), show(tv), tc.displayName()));
    }
  }
  void allocAndCopyROM(const uint8_t* source, size_t numBytes) override {
    if (numBytes == 0) return;
    auto const actualSize = numBytes;
    auto const index = MemoryManager::size2Index(actualSize);
    auto const allocedSize = MemoryManager::sizeIndex2Size(index);
    auto const overAlloced = allocedSize - actualSize;
    m_buf = (uint8_t*)tl_heap->mallocSmallIndexSize(index, allocedSize);
    memcpy(m_buf, source, actualSize);
    tl_heap->freeOveralloc(m_buf + actualSize, overAlloced);
  }
  void fixupInterior(ptrdiff_t fixupAddr, ptrdiff_t targetAddr) override {
    auto dest = (HeapObject**)(m_buf + fixupAddr);
    *dest = (HeapObject*)(m_buf + targetAddr);
  }
  void breakMystery(const MysteryBoxProvenance& provenance,
                    tv_val_offset tv_off) override {
    auto const tv = selectMysteryBox(provenance);
    auto const lval = tv_off.apply((char*)m_buf);
    lval.type() = tv.type();
    lval.val() = tv.val();
  }
  void incref(const MysteryBoxProvenance& provenance) override {
    auto const tv = selectMysteryBox(provenance);
    tvIncRefGen(tv);
  }
  void outputMystery(
      const MysteryBoxProvenance& provenance, uint32_t root) override {
    auto const tv = selectMysteryBox(provenance);
    roots.resize(root + 1);
    roots[root] = tv;
  }
  void outputHeapPtr(DataType dt, ptrdiff_t off, uint32_t root) override {
    roots.resize(root + 1);
    type(roots[root]) = dt;
    val(roots[root]).num = (int64_t)(m_buf + off);
  }
  void outputConstant(TypedValue tv, uint32_t root) override {
    roots.resize(root + 1);
    roots[root] = tv;
  }
  std::vector<TypedValue> roots;

private:
  TypedValue& selectMysteryBox(const MysteryBoxProvenance& provenance) {
    switch (provenance.type) {
      case MysteryBoxType::Unknown:
        always_assert(false);
      case MysteryBoxType::Context:
        return ctx;
      case MysteryBoxType::Local: {
        auto const local = provenance.local;
        if (local >= nargs) {
          throw RenderException(folly::sformat(
              "MysteryBox local {} out of range (nargs = {})", local, nargs));
        }
        return args[local];
      }
    }
    always_assert(false);
  }

  TypedValue ctx;
  uint32_t nargs;
  TypedValue* args;
  alignas(16) uint8_t* m_buf;
  bool partial;
};

}

struct ROMData {
  std::vector<TypedValue> roots;
  std::vector<MysteryBoxData> mysteries;
  std::vector<uint8_t> data;

  explicit ROMData(std::vector<TypedValue> tvs,
                   const std::vector<const MysteryBoxData*>& boxes)
    : roots{tvs}
  {
    build(boxes);
  }

  ROMData(const ROMData&) = delete;
  ROMData& operator=(const ROMData&) = delete;

  std::string info() const {
    if (debug) {
      checkInvariants();
    }
    uint64_t objects = 0;
    uint64_t fixups = 0;
    uint64_t placements = 0;

    Scanner countScanner([&](tv_lval tv) {
      if (isResourceType(tv.type())) {
        placements++;
      }
    }, [&](HeapObject** dptr) {
      HeapObject* obj = *dptr;
      if (!heapObjIsStatic(obj)) fixups++;
    }, [&](HeapObject*) {
      objects++;
    });

    Tracer tracer([&](HeapObject* obj) {
      countScanner.scan(obj);
    });
    for (auto root : roots) {
      countScanner.scan(&root);
      tracer.enqueue(&root);
    }
    tracer.trace();
    return folly::sformat(
      "size:{} objects:{} fixups: {} mysteries:{} placements:{}",
      data.size(), objects, fixups, mysteries.size(), placements);
  }
  std::string toString() const {
    if (debug) {
      checkInvariants();
    }
    int indent = 0;
    struct Indent {
      explicit Indent(int& indent) : m_indent(indent) { m_indent++; }
      ~Indent() { m_indent--; }
      int& m_indent;
    };
    std::ostringstream out;
    auto const print = [&](std::string s) {
      for (int i = 0; i < indent; i++) {
        out << "  ";
      }
      out << s << std::endl;
    };

    print("Mysteries:");
    {
      Indent _(indent);
      for (auto const& mystery : mysteries) {
        print(show(mystery.provenance));
      }
    }

    auto const toOffset = [&](HeapObject* ptr) {
      auto const off = reinterpret_cast<uint8_t*>(ptr) - data.data();
      assertx(off < data.size());
      return off;
    };
    auto const printHeapObj = [&](HeapObject* obj) {
      print(std::string("HeaderKind: ") + header_names[int(obj->kind())]);
      if (!heapObjIsStatic(obj)) {
        print(std::string("Offset: ") + folly::to<std::string>(toOffset(obj)));
      }
    };

    Scanner strScanner([&](tv_lval tv) {
      if (isResourceType(tv.type())) {
        auto const& data = mysteries[tv.val().num];
        print(folly::sformat("MysteryBox: {}", show(data.provenance)));
      }
      print(std::string("Type: ") + tname(tv.type()));
      return Indent(indent);
    }, [&](HeapObject** obj) {
      printHeapObj(*obj);
    }, [&](HeapObject* obj) {
      printHeapObj(obj);
      return Indent(indent);
    });

    Tracer tracer([&](HeapObject* obj) {
      strScanner.scan(obj);
    });
    for (auto root : roots) {
      strScanner.scan(&root);
      tracer.enqueue(&root);
    }
    tracer.trace();
    out << std::endl;
    return out.str();
  }

  void render(ROMRenderer& renderer) const {
    if (debug) {
      checkInvariants();
    }
    for (auto const& mb : mysteries) {
      for (auto const& c : mb.constraints) {
        renderer.checkConstraint(mb.provenance, c);
      }
    }

    auto base = data.data();
    auto size = data.size();
    if (size > 0) {
      renderer.allocAndCopyROM(base, size);
    }

    std::vector<uint32_t> mysteryCounts(mysteries.size(), 0);
    bool processingRoots = false;
    bool rootIsConstant = true;
    uint32_t rootIdx = 0;
    Scanner fixupScanner([&](tv_lval tv) {
      if (!isResourceType(tv.type())) return;

      auto const& mystery = mysteries[tv.val().num];
      mysteryCounts[tv.val().num]++;
      if (processingRoots) {
        rootIsConstant = false;
        renderer.outputMystery(mystery.provenance, rootIdx);
      } else {
        tv_val_offset tv_off {
          (uint8_t*)&tv.type() - base,
          (uint8_t*)&tv.val() - base
        };
        renderer.breakMystery(mystery.provenance, tv_off);
      }
    }, [&](HeapObject** obj) {
      auto const fixupPtr = (uint8_t*)(obj);
      auto const targetPtr = (uint8_t*)(*obj);
      if (targetPtr < base || base + size <= targetPtr) return;

      if (processingRoots) {
        // The fixup must be in the ROM's roots.  We haven't started scanning
        // yet.
        rootIsConstant = false;
        uint32_t idx = rootIdx;
        renderer.outputHeapPtr(roots[idx].type(), targetPtr - base, idx);
      } else{
        // If the fixup is not a root i must be a interior fixup.
        assertx(base <= fixupPtr && fixupPtr < base + size);
        renderer.fixupInterior(fixupPtr - base, targetPtr - base);
      }
    }, [](HeapObject*) {});


    Tracer tracer([&](HeapObject* obj) {
      fixupScanner.scan(obj);
    });
    for (; rootIdx < roots.size(); rootIdx++) {
      auto root = roots[rootIdx];
      tracer.enqueue(&root);
    }

    tracer.trace();

    processingRoots = true;
    for (rootIdx = 0; rootIdx < roots.size(); rootIdx++) {
      auto root = roots[rootIdx];
      fixupScanner.scan(&root);
      if (rootIsConstant) {
        renderer.outputConstant(root, rootIdx);
      }
      rootIsConstant = true;
    }

    for (uint32_t i = 0; i < mysteries.size(); i++) {
      auto const count = mysteryCounts[i];
      for (uint32_t rc = 0; rc < count; rc++) {
        renderer.incref(mysteries[i].provenance);
      }
    }
  }

private:
  void build(const std::vector<const MysteryBoxData*>& boxes) {
    struct Fixup {
      size_t size;
      ptrdiff_t off;
    };
    std::map<HeapObject*, Fixup, std::greater<HeapObject*>> fixupMap;
    hphp_fast_map<const MysteryBoxData*, uint64_t> mysteryMap;

    for (auto const& box : boxes) {
      auto const [_, inserted] = mysteryMap.insert({box, mysteries.size()});
      if (inserted) {
        mysteries.push_back(*box);
      }
    }

    Scanner copyScanner([&](tv_lval tv) {
      if (!isResourceType(tv.type())) return;
      UNUSED auto const box = unbox(tv.tv());
      assertx(box);
      assertx(mysteryMap.count(box));
    }, [](HeapObject**) {
    }, [&](HeapObject* obj) {
      auto const size = allocSize(obj);
      ptrdiff_t frontier = data.size();
      auto const [_, inserted] = fixupMap.insert({obj, {size, frontier}});

      if (inserted) {
        // TODO alignment of heap objects based on size classes.
        data.resize(data.size() + size);
        memcpy(&data[frontier], obj, size);
      }
    });
    Tracer copyTracer([&](HeapObject* obj) {
      copyScanner.scan(obj);
    });
    for (auto& root : roots) {
      copyScanner.scan(&root);
      copyTracer.enqueue(&root);
    }
    copyTracer.trace();

    auto const toPtr = [&](ptrdiff_t off) {
      return reinterpret_cast<HeapObject*>(data.data() + off);
    };
    Scanner fixupScanner([&](tv_lval tv) {
      if (!isResourceType(tv.type())) return;
      auto const box = unbox(tv.tv());
      assertx(box);
      auto const it = mysteryMap.find(box);
      assertx(it != mysteryMap.end());
      tv.val().num = it->second;
    }, [&](HeapObject** obj) {
      UNUSED bool isStatic = heapObjIsStatic(*obj);
      auto const it = fixupMap.lower_bound(*obj);
      if (it == fixupMap.end()) {
        assertx(isStatic);
        return;
      }
      ptrdiff_t off = reinterpret_cast<uintptr_t>(*obj) -
                      reinterpret_cast<uintptr_t>(it->first);
      if (off >= it->second.size) {
        assertx(isStatic);
        return;
      }
      off += it->second.off;

      *obj = toPtr(off);
    }, [](HeapObject*) {});
    Tracer fixupTracer([&](HeapObject* obj) {
      fixupScanner.scan(obj);
    });
    for (auto& root : roots) {
      fixupScanner.scan(&root);
      fixupTracer.enqueue(&root);
    }
    fixupTracer.trace();
    if (debug) {
      checkInvariants();
    }
  }
  void checkInvariants() const {
    auto const checkObj = [this] (HeapObject* obj) {
      auto const ptr = reinterpret_cast<uintptr_t>(obj);
      auto const base = reinterpret_cast<uintptr_t>(data.data());
      bool isUncounted = heapObjIsUncounted(obj);
      bool isStatic = heapObjIsStatic(obj);
      if (base <= ptr && ptr < base + data.size()) {
        always_assert(!isStatic);
      } else {
        always_assert(isStatic);
      }
      always_assert(!isUncounted);
    };
    Scanner invariantScanner([&](tv_lval tv) {
      if (!isResourceType(tv.type())) return;
      auto const mysteryNum = tv.val().num;
      always_assert(0 <= mysteryNum && mysteryNum < mysteries.size());
    }, [&](HeapObject** obj) {
      checkObj(*obj);
    }, [&](HeapObject* obj) {
      checkObj(obj);
    });
    Tracer tracer([&](HeapObject* obj) {
      invariantScanner.scan(obj);
    });
    for (auto root : roots) {
      invariantScanner.scan(&root);
      tracer.enqueue(&root);
    }
    tracer.trace();
  }
};

ROMHandle::ROMHandle() : data(nullptr) {}
ROMHandle::ROMHandle(std::unique_ptr<ROMData>&& data) : data(std::move(data)) {}
ROMHandle::ROMHandle(ROMHandle&&) noexcept = default;
ROMHandle& ROMHandle::operator =(ROMHandle&&) noexcept = default;
ROMHandle::~ROMHandle() = default;

IMPLEMENT_RESOURCE_ALLOCATION(ROMResourceHandle);

void renderROM(ROMRenderer& renderer, const ROMData& rom) {
  FTRACE(1, "ROM: \n{}", rom.toString());
  rom.render(renderer);
}

std::vector<TypedValue> renderROMOnce(const ROMData& rom, TypedValue ctx,
                                      std::vector<TypedValue>&& args,
                                      bool partial) {
  if (partial) {
    for (auto& mystery : rom.mysteries) {
      auto const checkMystery = [&] (TypedValue& tv) {
        if (!unbox(tv)) return;
        // Nothing to see here... this const cast is 100% legit.
        auto mbox = req::make<MysteryBox>(
          const_cast<MysteryBoxData*>(&mystery), true);

        tvCopy(make_tv<KindOfResource>(mbox.detach()->hdr()), tv);
      };
      switch (mystery.provenance.type) {
        case MysteryBoxType::Unknown:
          always_assert(false);
        case MysteryBoxType::Context:
          checkMystery(ctx);
          break;
        case MysteryBoxType::Local: {
          auto const local = mystery.provenance.local;
          if (local >= args.size()) {
            throw RenderException(folly::sformat(
                "MysteryBox local {} out of range (nargs = {})",
                local, args.size()));
          }
          checkMystery(args[local]);
          break;
        }
      }
    }
  }
  RenderOnce renderer(ctx, args.size(), args.data(), partial);
  renderROM(renderer, rom);
  return renderer.roots;
}

void makeStatic(std::vector<TypedValue>& roots) {
  hphp_fast_map<HeapObject*, HeapObject*> fixupMap;
  bool membersAreStatic;
  struct ScopedDecrefAndPersistence {
    explicit ScopedDecrefAndPersistence(tv_lval lval)
      : save(*lval), ref(lval)
    {}
    ~ScopedDecrefAndPersistence() {
      assertx(save.type() == ref.type());
      if (save.val().num == ref.val().num) return;
      tvDecRefGen(save);
      // We only mutate tvs in this pass to make their pointed to object
      // persistent.
      ref.type() = dt_with_persistence(ref.type());
    }
    TypedValue save;
    tv_lval ref;
  };
  Scanner staticificationScanner(
    [&](tv_lval lval) {
      if (unbox(*lval)) membersAreStatic = false;
      return ScopedDecrefAndPersistence(lval);
    },
    [&](HeapObject** obj) {
      if (heapObjIsStatic(*obj)) return;
      auto const it = fixupMap.find(*obj);
      if (it == fixupMap.end()) {
        membersAreStatic = false;
        return;
      }
      *obj = it->second;
    },
    [&](HeapObject*) {}
  );

  Tracer staticificationTracer([&](HeapObject* obj) {
    return POTracer([&](HeapObject* obj) {
      membersAreStatic = true;
      staticificationScanner.scan(obj);
      if (!membersAreStatic) return;
      auto const finish = [&](HeapObject* newObj) {
        UNUSED auto const [_, inserted] = fixupMap.try_emplace(obj, newObj);
        assertx(inserted);
      };

      auto const headerName = header_names[int(obj->kind())];
      switch (obj->kind()) {
        case HeaderKind::String:
          finish(makeStaticString(static_cast<StringData*>(obj)));
          break;
        case HeaderKind::Vec:
        case HeaderKind::Dict:
        case HeaderKind::Keyset:
          finish(ArrayData::GetScalarArray(Array(static_cast<ArrayData*>(obj))));
          break;
        case HeaderKind::BespokeVec:
        case HeaderKind::BespokeDict:
        case HeaderKind::BespokeKeyset: {
          auto ad = static_cast<ArrayData*>(obj);
          auto const layout = jit::ArrayLayout::FromArray(ad);
          assertx(layout.vanilla() || layout.bespokeLayout()->isConcrete());
          ad = ArrayData::GetScalarArray(Array(ad));
          auto const bad = layout.bespokeLayout()->coerce(ad);
          if (bad) {
            finish(bad);
          } else {
            finish(ad);
          }
          break;
        }
        case HeaderKind::ClsMeth:
        case HeaderKind::RClsMeth:
        case HeaderKind::RFunc:
          // TODO: Handle above trash (T29639296)
        case HeaderKind::AsyncFuncWH:
        case HeaderKind::AwaitAllWH:
        case HeaderKind::ConcurrentWH:
        case HeaderKind::NativeData:
        case HeaderKind::AsyncFuncFrame:
        case HeaderKind::Resource:
        case HeaderKind::NativeObject:
        case HeaderKind::Vector:
        case HeaderKind::Map:
        case HeaderKind::Set:
        case HeaderKind::Pair:
        case HeaderKind::ImmVector:
        case HeaderKind::ImmMap:
        case HeaderKind::ImmSet:
        case HeaderKind::MemoData:
        case HeaderKind::Object:
        case HeaderKind::Closure:
        case HeaderKind::ClosureHdr:
        case HeaderKind::WaitHandle:
          break;
        case HeaderKind::Cpp:
        case HeaderKind::SmallMalloc:
        case HeaderKind::BigMalloc:
        case HeaderKind::Free:
        case HeaderKind::Hole:
        case HeaderKind::Slab:
          always_assert_flog(false,
                             "Hit unhandled heap object kind during "
                             "staticification {}", headerName);
      }
    }, obj);
  });
  for (auto& root : roots) {
    staticificationTracer.enqueue(&root);
  }
  staticificationTracer.trace();
  for (auto& root : roots) {
    staticificationScanner.scan(&root);
  }
}

ROMHandle optimizeROM(ROMHandle&& rom, bool& unwrapEager, TypedValue ctx,
                      uint32_t nargs, const TypedValue* args) {
  FTRACE(1, "Optimize ROM\n");
  auto roots = renderROMOnce(
    rom.get(), ctx,
    std::vector<TypedValue>(args, args + nargs),
    true);
  makeStatic(roots);
  assertx(roots.size() == 1);
  if (unwrapEager) {
    auto const awaitable = roots[0];
    auto wh = c_Awaitable::fromTV(awaitable);
    if (wh && wh->isSucceeded()) {
      auto const result = wh->getResult();
      auto newRom = computeROM({result}, &rom.get());
      if (newRom) {
        return std::move(*newRom);
      }
    }
  }
  unwrapEager = false;
  auto newRom = computeROM(roots, &rom.get());
  if (newRom) {
    return std::move(*newRom);
  }
  return std::move(rom);
}

Optional<ROMHandle> computeROM(const std::vector<TypedValue>& roots,
                               const ROMData* baseROM) {
  std::vector<const MysteryBoxData*> boxes;
  for (auto const& box : baseROM->mysteries) {
    boxes.push_back(&box);
  }
  try {
    ROMHandle rom(std::make_unique<ROMData>(roots, boxes));
    FTRACE(1, "ROM: \n{}", rom.data->toString());
    return Optional<ROMHandle>(std::move(rom));
  } catch (const std::exception& exn) {
    FTRACE(1, "Caught C++ exception: {}\n", exn.what());
  }
  return std::nullopt;
}

std::string show(const ROMData& rom) {
  return rom.toString();
}

std::string showShort(const ROMData& rom) {
  return rom.info();
}

//////////////////////////////////////////////////////////////////////////////

namespace {

SrcKey vmsk() {
  return SrcKey(vmfp()->func(), vmpc(), resumeModeFromActRec(vmfp()));
}

// Returns true if the given operation may read or write any global VM state
// (including memo caches, static props, etc.). This check only checks for
// "direct" global effects - for example, BaseGC / BaseSC member ops - and not
// global effects due to VM re-entry.
//
// We don't need to worry about re-entry (for errors or for the "object" case
// of an op like IterInit) because those effects are detected by other means:
//
//  1. We set ThrowAllErrorsSetter and catch errors in our interpreter loop.
//
//  2. We set g_context->inlineInputState to START; on entry, the interpreter
//     sets this state to BLOCK, so that any further re-entry is an error.
//
// TODO(kshaunak): Do the casework here.
//
bool mayReadOrWriteGlobals(Op op) {
  switch (op) {
    case Op::FuncCred:
    case Op::Print:
    case Op::Clone:
    case Op::Exit:
    case Op::Fatal:
    case Op::CnsE:
    case Op::CGetG:
    case Op::CGetS:
    case Op::IssetG:
    case Op::IssetS:
    case Op::SetG:
    case Op::SetS:
    case Op::SetOpG:
    case Op::SetOpS:
    case Op::IncDecG:
    case Op::IncDecS:
    case Op::UnsetG:
    case Op::Incl:
    case Op::InclOnce:
    case Op::Req:
    case Op::ReqOnce:
    case Op::ReqDoc:
    case Op::Eval:
    case Op::NativeImpl:
    case Op::ContEnter:
    case Op::ContRaise:
    case Op::Yield:
    case Op::YieldK:
    case Op::ContCheck:
    case Op::ContValid:
    case Op::ContKey:
    case Op::ContCurrent:
    case Op::ContGetReturn:
    case Op::Await:
    case Op::AwaitAll:
    case Op::Silence:
    case Op::BaseGC:
    case Op::BaseGL:
    case Op::BaseSC:
    case Op::MemoGet:
    case Op::MemoGetEager:
    case Op::MemoSet:
    case Op::MemoSetEager:
    case Op::SetImplicitContextByValue:
    case Op::CreateSpecialImplicitContext:
      return true;

    case Op::Nop:
    case Op::BreakTraceHint:
    case Op::PopC:
    case Op::PopU:
    case Op::PopU2:
    case Op::PopL:
    case Op::Dup:
    case Op::CGetCUNop:
    case Op::UGetCUNop:
    case Op::Null:
    case Op::NullUninit:
    case Op::True:
    case Op::False:
    case Op::Int:
    case Op::Double:
    case Op::String:
    case Op::Dict:
    case Op::Keyset:
    case Op::Vec:
    case Op::NewDictArray:
    case Op::NewStructDict:
    case Op::NewVec:
    case Op::NewKeysetArray:
    case Op::AddElemC:
    case Op::AddNewElemC:
    case Op::NewCol:
    case Op::NewPair:
    case Op::ColFromArray:
    case Op::ClsCns:
    case Op::ClsCnsD:
    case Op::ClsCnsL:
    case Op::ClassName:
    case Op::LazyClassFromClass:
    case Op::File:
    case Op::Dir:
    case Op::Method:
    case Op::Concat:
    case Op::ConcatN:
    case Op::Add:
    case Op::Sub:
    case Op::Mul:
    case Op::Div:
    case Op::Mod:
    case Op::Pow:
    case Op::Not:
    case Op::Same:
    case Op::NSame:
    case Op::Eq:
    case Op::Neq:
    case Op::Lt:
    case Op::Lte:
    case Op::Gt:
    case Op::Gte:
    case Op::Cmp:
    case Op::BitAnd:
    case Op::BitOr:
    case Op::BitXor:
    case Op::BitNot:
    case Op::Shl:
    case Op::Shr:
    case Op::CastBool:
    case Op::CastInt:
    case Op::CastDouble:
    case Op::CastString:
    case Op::CastDict:
    case Op::CastKeyset:
    case Op::CastVec:
    case Op::DblAsBits:
    case Op::InstanceOf:
    case Op::InstanceOfD:
    case Op::IsLateBoundCls:
    case Op::IsTypeStructC:
    case Op::ThrowAsTypeStructException:
    case Op::CombineAndResolveTypeStruct:
    case Op::Select:
    case Op::Enter:
    case Op::Jmp:
    case Op::JmpZ:
    case Op::JmpNZ:
    case Op::Switch:
    case Op::SSwitch:
    case Op::RetC:
    case Op::RetM:
    case Op::RetCSuspended:
    case Op::Throw:
    case Op::CGetL:
    case Op::CGetQuietL:
    case Op::CUGetL:
    case Op::CGetL2:
    case Op::PushL:
    case Op::ClassGetC:
    case Op::ClassGetTS:
    case Op::GetMemoKeyL:
    case Op::AKExists:
    case Op::IssetL:
    case Op::IsUnsetL:
    case Op::IsTypeC:
    case Op::IsTypeL:
    case Op::AssertRATL:
    case Op::AssertRATStk:
    case Op::SetL:
    case Op::SetOpL:
    case Op::IncDecL:
    case Op::UnsetL:
    case Op::ResolveFunc:
    case Op::ResolveMethCaller:
    case Op::ResolveRFunc:
    case Op::ResolveClsMethod:
    case Op::ResolveClsMethodD:
    case Op::ResolveClsMethodS:
    case Op::ResolveRClsMethod:
    case Op::ResolveRClsMethodD:
    case Op::ResolveRClsMethodS:
    case Op::ResolveClass:
    case Op::LazyClass:
    case Op::NewObj:
    case Op::NewObjD:
    case Op::NewObjS:
    case Op::LockObj:
    case Op::FCallClsMethod:
    case Op::FCallClsMethodM:
    case Op::FCallClsMethodD:
    case Op::FCallClsMethodS:
    case Op::FCallClsMethodSD:
    case Op::FCallCtor:
    case Op::FCallFunc:
    case Op::FCallFuncD:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD:
    case Op::IterInit:
    case Op::LIterInit:
    case Op::IterNext:
    case Op::LIterNext:
    case Op::IterFree:
    case Op::LIterFree:
    case Op::This:
    case Op::BareThis:
    case Op::CheckThis:
    case Op::ChainFaults:
    case Op::OODeclExists:
    case Op::VerifyImplicitContextState:
    case Op::VerifyOutType:
    case Op::VerifyParamType:
    case Op::VerifyParamTypeTS:
    case Op::VerifyRetTypeC:
    case Op::VerifyRetTypeTS:
    case Op::VerifyRetNonNullC:
    case Op::SelfCls:
    case Op::ParentCls:
    case Op::LateBoundCls:
    case Op::RecordReifiedGeneric:
    case Op::CheckClsReifiedGenericMismatch:
    case Op::ClassHasReifiedGenerics:
    case Op::GetClsRGProp:
    case Op::HasReifiedParent:
    case Op::CheckClsRGSoft:
    case Op::CreateCl:
    case Op::CreateCont:
    case Op::WHResult:
    case Op::Idx:
    case Op::ArrayIdx:
    case Op::ArrayMarkLegacy:
    case Op::ArrayUnmarkLegacy:
    case Op::CheckProp:
    case Op::InitProp:
    case Op::ThrowNonExhaustiveSwitch:
    case Op::RaiseClassStringConversionWarning:
    case Op::BaseL:
    case Op::BaseC:
    case Op::BaseH:
    case Op::Dim:
    case Op::QueryM:
    case Op::SetM:
    case Op::SetRangeM:
    case Op::IncDecM:
    case Op::SetOpM:
    case Op::UnsetM:
      return false;
  }

  always_assert_flog(0, "Invalid op: {}\n", static_cast<uint32_t>(op));
}

bool allowBoxesInLocalInputs(Op op) {
  switch (op) {
    case Op::CGetL:
    case Op::CGetQuietL:
    case Op::CUGetL:
    case Op::CGetL2:
    case Op::PopL:
    case Op::PushL:
    case Op::SetL:
    case Op::UnsetL:
    case Op::VerifyParamType:
      return true;
    default:
      return false;
  }
}

bool allowBoxesInStackInputs(Op op) {
  switch (op) {
    case Op::CGetL2:
    case Op::CGetCUNop:
    case Op::UGetCUNop:
    case Op::Dup:
    case Op::PopC:
    case Op::PopL:
    case Op::PopU:
    case Op::PopU2:
    case Op::SetL:
    case Op::SetM:
    case Op::NewVec:
    case Op::NewStructDict:
    case Op::NewKeysetArray:
    case Op::NewPair:
    case Op::VerifyRetTypeC:
    case Op::VerifyRetNonNullC:
    case Op::FCallClsMethod:
    case Op::FCallClsMethodD:
    case Op::FCallClsMethodS:
    case Op::FCallClsMethodSD:
    case Op::FCallCtor:
    case Op::FCallFunc:
    case Op::FCallFuncD:
    case Op::FCallObjMethod:
    case Op::FCallObjMethodD:
    case Op::RetC:
    case Op::RetM:
      return true;
    default:
      return false;
  }
}

enum class ThisPointerEffect { NONE, READ_CLASS, READ_VALUE };

ThisPointerEffect getThisPointerEffect(SrcKey sk) {
  using TPE = ThisPointerEffect;
  auto const op = sk.op();
  auto const reads_this = [](const TypeConstraint& tc) {
    return tc.isCheckable() && tc.isThis();
  };

  switch (op) {
    case Op::BaseH:
    case Op::This:
    case Op::BareThis:
    case Op::CheckThis:
      return TPE::READ_VALUE;

    case Op::ContCheck:
    case Op::ContCurrent:
    case Op::ContEnter:
    case Op::ContGetReturn:
    case Op::ContKey:
    case Op::ContRaise:
    case Op::ContValid:
      return TPE::READ_VALUE;

    case Op::NewObjS:
      return TPE::READ_VALUE;

    case Op::FCallClsMethod:
    case Op::FCallClsMethodD:
    case Op::FCallClsMethodS:
    case Op::FCallClsMethodSD:
    case Op::ResolveClsMethodS:
    case Op::ResolveRClsMethodS: {
      if (isFCall(op)) {
        auto const fca = getImm(sk.pc(), 0, sk.unit()).u_FCA;
        if (!fca.context || liveHasThis()) return TPE::READ_CLASS;
      }
      if (op == OpFCallClsMethod || op == OpFCallClsMethodD) return TPE::NONE;
      auto const idx = isFCall(op) ? 2 : 0;
      auto const raw = getImm(sk.pc(), idx, sk.unit()).u_OA;
      auto const ref = static_cast<SpecialClsRef>(raw);
      return ref == SpecialClsRef::LateBoundCls ? TPE::READ_CLASS : TPE::NONE;
    }

    case Op::LateBoundCls:
    case Op::IsLateBoundCls:
      return TPE::READ_CLASS;

    case Op::VerifyOutType:
    case Op::VerifyParamType: {
      auto const func = vmfp()->func();
      auto const local = op == Op::VerifyParamType
        ? getImm(sk.pc(), 0).u_ILA
        : getImm(sk.pc(), 0).u_IVA;
      assertx(0 <= local && local < func->numParams());
      auto const tc = func->params()[local].typeConstraint;
      if (reads_this(tc)) return TPE::READ_CLASS;
      if (func->hasParamsWithMultiUBs()) {
        auto const& ubs = func->paramUBs();
        auto const it = ubs.find(local);
        if (it != ubs.end()) {
          for (auto const& ub : it->second.m_constraints) {
            if (reads_this(ub)) return TPE::READ_CLASS;
          }
        }
      }
      return TPE::NONE;
    }

    case Op::VerifyRetTypeC:
    case Op::VerifyRetNonNullC: {
      auto const func = vmfp()->func();
      auto const tc = func->returnTypeConstraint();
      if (reads_this(tc)) return TPE::READ_CLASS;
      if (op != Op::VerifyRetNonNullC && func->hasReturnWithMultiUBs()) {
        auto const& ubs = func->returnUBs();
        for (auto const& ub : ubs.m_constraints) {
          if (reads_this(ub)) return TPE::READ_CLASS;
        }
      }
      return TPE::NONE;
    }

    case Op::CombineAndResolveTypeStruct:
    case Op::IsTypeStructC:
    case Op::ThrowAsTypeStructException:
    case Op::VerifyParamTypeTS:
    case Op::VerifyRetTypeTS:
      return TPE::READ_CLASS;

    case Op::MemoGet:
    case Op::MemoGetEager:
    case Op::MemoSet:
    case Op::MemoSetEager: {
      auto const func = sk.func();
      auto const instance_method = func->isMethod() && !func->isStatic();
      return instance_method ? TPE::READ_VALUE : TPE::NONE;
    }

    default:
      return TPE::NONE;
  }
}

struct HHBCMemoryEffects {
  bool pessimize = false;
  bool readsMemberBase = false;
  MemberKey memberKey = {};
  req::TinyVector<uint32_t, 4> locals;
  req::TinyVector<LocalRange, 2> localRanges;
  ThisPointerEffect thisPointerEffect = ThisPointerEffect::NONE;
  uint32_t stackPops = 0;
};

HHBCMemoryEffects computeMemoryEffects(SrcKey sk) {
  auto const op = sk.op();
  auto result = HHBCMemoryEffects{};
  if (mayReadOrWriteGlobals(op)) {
    result.pessimize = true;
    return result;
  }

  if (isMemberDimOp(op) || isMemberFinalOp(op)) {
    result.readsMemberBase = true;
  }

  auto const pc = sk.pc();
  auto const unit = sk.unit();
  result.stackPops = instrNumPops(pc);
  result.thisPointerEffect = getThisPointerEffect(sk);

  auto const local = [&](auto i) {
    result.locals.push_back(safe_cast<uint32_t>(i));
  };

#define ONE(a)                H_##a(0);
#define TWO(a, b)             H_##a(0); H_##b(1);
#define THREE(a, b, c)        H_##a(0); H_##b(1); H_##c(2);
#define FOUR(a, b, c, d)      H_##a(0); H_##b(1); H_##c(2); H_##d(3);
#define FIVE(a, b, c, d, e)   H_##a(0); H_##b(1); H_##c(2); H_##d(3); H_##e(4);
#define SIX(a, b, c, d, e, f) H_##a(0); H_##b(1); H_##c(2); H_##d(3); H_##e(4); H_##f(5);

#define H_LA(i)    local(getImm(pc, i, unit).u_LA)
#define H_ILA(i)   local(getImm(pc, i, unit).u_ILA)
#define H_NLA(i)   local(getImm(pc, i, unit).u_NLA.id)
#define H_KA(i)    result.memberKey = getImm(pc, i, unit).u_KA
#define H_LAR(i)   result.localRanges.push_back(getImm(pc, i, unit).u_LAR)
#define H_IVA(i)   // variable-size integer
#define H_I64A(i)  // 64-bit integer
#define H_IA(i)    // iterator ID
#define H_DA(i)    // double
#define H_BA(i)    // bytecode offset
#define H_OA(i)    // sub-opcode enum
#define H_SA(i)    // string literal ID
#define H_RATA(i)  // repo-authoritative type
#define H_AA(i)    // static array literal ID
#define H_BLA(i)   // bytecode offset vector
#define H_SLA(i)   // string literal ID and offset pair
#define H_VSA(i)   // string literal ID vector
#define H_ITA(i)   // IterArgs; we do not track writes to iter output locals
#define H_FCA(i)   // FCallArgs;

  switch (op) {
#define O(name, imm, push, pop, flags) case Op##name: imm; break;
    OPCODES
#undef O
  }

#undef H_BLA
#undef H_SLA
#undef H_IVA
#undef H_I64A
#undef H_LA
#undef H_NLA
#undef H_ILA
#undef H_IA
#undef H_DA
#undef H_BA
#undef H_OA
#undef H_SA
#undef H_RATA
#undef H_AA
#undef H_VSA
#undef H_KA
#undef H_LAR
#undef H_ITA
#undef H_FCA

#undef ONE
#undef TWO
#undef THREE
#undef FOUR
#undef FIVE
#undef SIX

  return result;
}

}

//////////////////////////////////////////////////////////////////////////////

namespace {
void hackilySetClassPointer(ObjectData* obj, const Class* cls) {
  auto const raw = reinterpret_cast<char*>(obj);
  auto constexpr offset = ObjectData::getVMClassOffset();
  *reinterpret_cast<LowPtr<Class>*>(raw + offset) = const_cast<Class*>(cls);
}
}

struct FakeObject {
  FakeObject(const FakeObject&) = delete;
  FakeObject& operator=(const FakeObject&) = delete;

  FakeObject(FakeObject&& o) noexcept : obj(o.obj) { o.obj = nullptr; }
  FakeObject& operator=(FakeObject&& o) noexcept {
    std::swap(obj, o.obj); return *this;
  }

  explicit FakeObject(const Class* cls) {
    auto const raw = reinterpret_cast<char*>(malloc(sizeof(ObjectData)));
    obj = reinterpret_cast<ObjectData*>(raw);
    obj->initHeader_16(HeaderKind::Object, OneReference, 0);
    hackilySetClassPointer(obj, cls);
  }

  ~FakeObject() {
    if (obj) free(obj);
  }

  ObjectData* obj = nullptr;

  TYPE_SCAN_IGNORE_ALL;
};

struct InlineInterpContext {
  InlineInterpContext(const InlineInterpContext&) = delete;
  InlineInterpContext& operator=(const InlineInterpContext&) = delete;

  InlineInterpContext(InlineInterpContext&&) = default;
  InlineInterpContext& operator=(InlineInterpContext&&) = default;

  InlineInterpContext() {}

  req::vector<req::ptr<MysteryBox>> boxes;
  req::vector<FakeObject> objects;
  req::fast_map<ObjectData*, ResourceHdr*> links;
  uint64_t numOps{0};

  TYPE_SCAN_IGNORE_FIELD(objects);
  TYPE_SCAN_IGNORE_FIELD(links);
};

namespace {

RDS_LOCAL(InlineInterpContext, rl_context);

template <typename Predicate>
bool tvContainsMatchingValue(TypedValue tv, Predicate pr) {
  auto result = false;
  Scanner scanner(
    [&](tv_lval lval) { if (!result && pr(*lval)) result = true; },
    [&](HeapObject**) {},
    [&](HeapObject*) {}
  );
  Tracer tracer([&](HeapObject* obj) {
    scanner.scan(obj);
  });
  scanner.scan(&tv);
  tracer.enqueue(&tv);
  tracer.trace();
  return result;
}

bool tvContainsBoxedValue(TypedValue tv) {
  return tvContainsMatchingValue(tv, [&](auto const x) {
    return unbox(x) != nullptr;
  });
}

bool tvContainsReffyValue(TypedValue tv) {
  return tvContainsMatchingValue(tv, [&](auto const x) {
    return isRefcountedType(x.type()) && !tvIsArrayLike(x) && !tvIsString(x);
  });
}

/*
 * Ensure that a local is ready for inline-interp. We only allow one of the
 * following two cases at this point:
 *
 *  1. The local is a MysteryBox. In this case, we annotate the box with its
 *     "provenance": it's one of the input locals for the inline-interp call.
 *
 *  2. The local is a live value. In order to maintain aliasing guarantees,
 *     we want to ensure that writes to this local can never mutate memory
 *     that is accessible from outside this context. There's a simple way to
 *     do so: we require that no live values contain reffy types. Arrays,
 *     strings, and primitives are okay; anything that recursively contains
 *     an object or resource is not.
 */
bool materializeBox(tv_lval tv, MysteryBoxProvenance provenance) {
  auto const box = unbox(*tv);
  if (!box) return !tvContainsReffyValue(*tv);
  auto const raw = new MysteryBoxData({provenance, box->type, {}});
  auto set = req::make<MysteryBox>(raw);
  tvCopy(make_tv<KindOfResource>(set->hdr()), tv);
  rl_context->boxes.push_back(std::move(set));
  return true;
}

/*
 * Materialize a FakeObject of the given class, overwriting the MysteryBox at
 * the given lval. The FakeObject only has a header and class pointer; it does
 * not have any of the object's properties or memo fields.
 */
void materializeObject(tv_lval lval, const Class* cls) {
  assertx(unbox(*lval));
  rl_context->objects.emplace_back(cls);
  auto const obj = rl_context->objects.back().obj;
  rl_context->links.insert({obj, lval.val().pres});
  tvSet(make_tv<KindOfObject>(obj), lval);
}

Optional<TypedValue> unboxThisPointer() {
  if (!vmfp()->func()->cls()) return std::nullopt;
  if (!vmfp()->hasThis()) return std::nullopt;
  auto const it = rl_context->links.find(vmfp()->getThis());
  if (it == rl_context->links.end()) return std::nullopt;
  auto const tv = make_tv<KindOfResource>(it->second);
  FTRACE(2, "    This pointer is a FakeObject for: {}\n", show(tv));
  return tv;
}

bool ensureThisPointerHasPreciseClass() {
  auto const tv = unboxThisPointer();
  if (!tv) return true;
  auto const box = unbox(*tv);
  assertx(box);
  auto const cls = box->type.clsSpec().exactCls();
  FTRACE(2, "    This op needs an exact class for $this; {}.\n",
         cls ? "continuing" : "stopping");
  if (!cls) return false;
  hackilySetClassPointer(vmfp()->getThis(), cls);
  return true;
}

/*
 * Certain bytecodes are too hard to analyze in general. Instead, we have
 * custom logic for them that checks for easy cases where we can continue
 * inline interpretation, and otherwise bails out.
 */
Optional<InlineInterpHookResult> tryCustomInlineInterpStep(SrcKey sk) {
  using IIHR = InlineInterpHookResult;
  auto const op = sk.op();
  switch (op) {
    case Op::AssertRATL:
    case Op::AssertRATStk: {
      auto const tv = sk.op() == Op::AssertRATL
        ? *frame_local(vmfp(), getImm(vmpc(), 0).u_ILA)
        : *vmStack().indTV(getImm(vmpc(), 0).u_IVA);
      if (!unbox(tv)) return std::nullopt;
      FTRACE(2, "    Skipping assertion on boxed input: {}\n", show(tv));
      vmpc() = sk.advanced().pc();
      return IIHR::SKIP;
    }
    case Op::Await: {
      auto const wh = c_Awaitable::fromTV(*vmStack().topC());
      auto const finished = wh && wh->isFinished();
      FTRACE(2, "    Got {}finished Awaitable.\n", finished ? "" : "un");
      return finished ? IIHR::NONE : IIHR::STOP;
    }
    case Op::AwaitAll: {
      auto const unfinished = []{
        auto result = 0;
        auto const range = getImm(vmpc(), 0).u_LAR;
        for (auto i = range.first; i < range.first + range.count; i++) {
          auto const tv = *frame_local(vmfp(), i);
          if (tvIsNull(tv)) continue;
          auto const wh = c_Awaitable::fromTV(tv);
          if (!(wh && wh->isFinished())) result++;
        }
        return result;
      }();
      FTRACE(2, "    Got {} unfinished Awaitable(s).\n", unfinished);
      return unfinished ? IIHR::STOP : IIHR::NONE;
    }
    case Op::CnsE: {
      auto const name = sk.unit()->lookupLitstrId(getImm(vmpc(), 0).u_SA);
      if (s_stdin.equal(name) || s_stdout.equal(name) || s_stderr.equal(name)) {
        FTRACE(2, "    Got stdin/stdout/stderr stream constant.\n");
        return IIHR::STOP;
      }
      auto const okay = Constant::lookupPersistent(name);
      FTRACE(2, "    Got {}persistent global constant.\n", okay ? "" : "non-");
      return okay ? IIHR::NONE : IIHR::STOP;
    }
    case Op::NativeImpl: {
      auto const func = vmfp()->func();
      assertx(func->isCPPBuiltin());
      FTRACE(2, "    Got {}foldable native function: {}\n",
             func->isFoldable() ? "" : "non-", func->fullName());
      for (auto i = 0; i < func->numParams(); i++) {
        auto const tv = *frame_local(vmfp(), i);
        if (tvContainsBoxedValue(tv)) {
          FTRACE(2, "    Local {} contains a boxed value: {}\n", i, show(tv));
          return IIHR::STOP;
        }
      }
      return func->isFoldable() ? IIHR::NONE : IIHR::STOP;
    }
    case Op::BaseH:
    case Op::This:
    case Op::BareThis:
    case Op::CheckThis: {
      if (op != Op::BaseH && !(vmfp()->func()->cls() && vmfp()->hasThis())) {
        return IIHR::NONE;
      }
      auto const tv = unboxThisPointer();
      if (!tv) return std::nullopt;
      if (op == Op::This || op == Op::BareThis) {
        tvDup(*tv, vmStack().allocC());
      } else if (op == Op::BaseH) {
        auto& mstate = vmMInstrState();
        mstate.tvTempBase = *tv;
        mstate.base = &mstate.tvTempBase;
        mstate.roProp = false;
      }
      vmpc() = sk.advanced().pc();
      return IIHR::SKIP;
    }
    case Op::NewObjS: {
      auto const tv = unboxThisPointer();
      if (!tv) return std::nullopt;
      auto const ref = static_cast<SpecialClsRef>(getImm(vmpc(), 0).u_OA);
      if (ref == SpecialClsRef::LateBoundCls && !ensureThisPointerHasPreciseClass()) {
        return IIHR::STOP;
      }
      auto const cls = specialClsRefToCls(ref);
      if (!cls->hasReifiedGenerics()) return std::nullopt;
      FTRACE(2, "    {} is reified and $this is boxed: {}\n",
             cls->name()->data(), show(*tv));
      return IIHR::STOP;
    }
    default: return std::nullopt;
  }
}

/*
 * After resolving an object method call with a boxed $this pointer, check if
 * we have enough information to actually invoke the function.
 */
bool allowBoxesInThisPointer(const Func* func, TypedValue ctx) {
  always_assert(unbox(ctx));
  if (CoeffectsConfig::enabled() && func->hasCoeffectRules()) {
    for (auto const& rule : func->getCoeffectRules()) {
      using Type = CoeffectRule::Type;
      auto const type = rule.m_type;
      if (type == Type::CCThis || type == Type::GeneratorThis ||
          type == Type::CCReified || type == Type::ClosureParentScope) {
        FTRACE(2, "    {} requires $this for coeffects rules; got: {}\n",
               func->fullName(), show(ctx));
        return false;
      }
    }
  }
  return true;
}

/*
 * Check that we have a specific-enough type for an object input when calling
 * a constructor or an instance method. If so, replace the boxed object input.
 */
bool checkForKnownInstanceMethod(SrcKey sk) {
  auto const op = sk.op();
  if (op != OpFCallCtor && op != OpFCallObjMethod && op != OpFCallObjMethodD) {
    return true;
  }

  auto const fca = getImm(sk.pc(), 0).u_FCA;
  auto const extra = op == Op::FCallObjMethod ? 1 : 0;
  auto const index = fca.numInputs() + (kNumActRecCells - 1) + extra;
  auto const lval = vmStack().indC(index);
  auto const box = unbox(*lval);

  if (!box) {
    FTRACE(2, "    Receiver is unboxed: {}\n", show(*lval));
    return true;
  } else if (auto const tv = box->type.tv()) {
    FTRACE(2, "    Receiver is a constant: {}\n", show(*lval));
    tvMove(*tv, lval);
    return true;
  }

  auto const ctx = vmfp() ? vmfp()->func()->cls() : nullptr;
  auto const callCtx = MemberLookupContext(ctx, vmfp()->func());
  auto const spec = box->type.clsSpec();
  auto const hint = sk.unit()->lookupLitstrId(getImm(sk.pc(), 1).u_SA);
  auto const hinted = !hint->empty()
    ? Class::lookupUniqueInContext(hint, ctx, nullptr)
    : nullptr;

  auto const func = [&]() -> const Func* {
    if (op == Op::FCallCtor) {
      return hinted ? lookupImmutableCtor(hinted, callCtx)
                    : lookupImmutableCtor(spec.exactCls(), callCtx);
    }
    auto const name = [&]{
      if (op == Op::FCallObjMethodD) {
        return sk.unit()->lookupLitstrId(getImm(sk.pc(), 3).u_SA);
      }
      auto const tv = *vmStack().topC();
      return tvIsString(tv) ? tv.val().pstr : nullptr;
    }();
    if (!name) return nullptr;
    auto const result = hinted
      ? lookupImmutableObjMethod(hinted, name, callCtx, true)
      : lookupImmutableObjMethod(spec.cls(), name, callCtx, spec.exact());
    if (result.type != ImmutableObjMethodLookup::Type::Func) return nullptr;
    if (result.func->isStaticInPrologue()) return nullptr;
    return result.func;
  }();

  if (!func) {
    FTRACE(2, "    Static dispatch failed: {}\n", show(*lval));
    return false;
  } else if (!allowBoxesInThisPointer(func, *lval)) {
    return false;
  }

  FTRACE(2, "    Static dispatch for {} succeeded: {}\n",
         show(*lval), func->fullName());
  materializeObject(lval, func->implCls());
  return true;
}

/*
 * The generic analysis for a bytecode is primarily concerned with identifying
 * the bytecode's "inputs" - locals and stack cells - and checking that these
 * locations don't contain any MysteryBox values.
 *
 * Certain bytecodes can operate correctly on boxed values. PushL is a simple
 * case, since it simply pushes the boxed local onto the stack. However, we
 * can even execute a NewVec or a SetM with boxed values. Doing so results in
 * MysteryBox values in an array's elements or an object's props.
 */
bool handledByInterpreter(SrcKey sk) {
  auto const op = sk.op();
  auto const effects = computeMemoryEffects(sk);
  if (effects.pessimize) {
    FTRACE(2, "    Unhandled global effect: {}\n", opcodeToName(op));
    return false;
  }

  auto const boxedLocal = [&](uint32_t id) {
    auto const tv = *frame_local(vmfp(), id);
    auto const boxed = unbox(tv);
    if (boxed) FTRACE(2, "    Local input @ {} is boxed: {}\n", id, show(tv));
    return boxed;
  };
  auto const boxedStack = [&](uint32_t off) {
    auto const tv = *vmStack().indTV(off);
    auto const boxed = unbox(tv);
    if (boxed) FTRACE(2, "    Stack input @ {} is boxed: {}\n", off, show(tv));
    return boxed;
  };

  if (effects.readsMemberBase) {
    auto const tv = *vmMInstrState().base;
    if (unbox(tv)) {
      FTRACE(2, "    Member base is boxed: {}\n", show(tv));
      return false;
    }
  }
  switch (effects.memberKey.mcode) {
    case MEC: case MPC:
      if (boxedStack(effects.memberKey.iva)) return false;
      break;
    case MEL: case MPL:
      if (boxedLocal(effects.memberKey.local.id)) return false;
      break;
    case MW: case MEI: case MET: case MPT: case MQT:
      break;
  }

  if (!allowBoxesInLocalInputs(op)) {
    for (auto const local : effects.locals) {
      if (boxedLocal(local)) return false;
    }
    for (auto const range : effects.localRanges) {
      for (auto i = 0; i < range.count; i++) {
        if (boxedLocal(range.first + i)) return false;
      }
    }
  }
  if (!allowBoxesInStackInputs(op)) {
    for (auto i = 0; i < effects.stackPops; i++) {
      if (boxedStack(i)) return false;
    }
  }

  switch (effects.thisPointerEffect) {
    case ThisPointerEffect::NONE: break;
    case ThisPointerEffect::READ_CLASS: {
      if (ensureThisPointerHasPreciseClass()) break;
      return false;
    }
    case ThisPointerEffect::READ_VALUE: {
      if (!unboxThisPointer()) break;
      FTRACE(2, "    This op needs a value for $this; stopping.\n");
      return false;
    }
  }

  return checkForKnownInstanceMethod(sk);
}

Optional<CallCtx> resolveCallContext(const Func* func, TypedValue ctx) {
  FTRACE(2, "  Context: {}\n", show(ctx));
  auto result = CallCtx { func, nullptr, nullptr, false };
  auto& context = tvAsVariant(ctx);
  auto const cls = func->implCls();

  if (context.isString()) {
    result.cls = Class::load(context.toString().get());
  } else if (context.isClass()) {
    result.cls = context.toClassVal();
  } else if (context.isLazyClass()) {
    result.cls = Class::load(context.toLazyClassVal().name());
  } else if (context.isNull()) {
  } else if (unbox(ctx)) {
    materializeBox(&ctx, {MysteryBoxType::Context});
    ctx.val().pres->incRefCount();
    auto const closure = func->isClosureBody();
    auto const box = unbox(ctx);
    if (!cls) {
      FTRACE(1, "Non-method function cannot take boxed context: {}\n",
             func->fullName()->data());
      return std::nullopt;
    } else if (closure) {
      FTRACE(1, "Cannot super-inline starting at a closure yet: {}\n",
             func->fullName()->data());
      return std::nullopt;
    }
    auto const refinement = closure ? jit::Type::ExactObj(cls)
                                    : jit::Type::SubObj(cls);
    auto const type = box->type & refinement;
    auto const type_cls = type.clsSpec().cls();
    if (!type_cls) {
      FTRACE(1, "Class type for {} context is unknown: {} -> {}\n",
             func->fullName()->data(), box->type.toString(), type.toString());
      return std::nullopt;
    } else if (!allowBoxesInThisPointer(func, ctx)) {
      return std::nullopt;
    }
    box->type = type;
    materializeObject(&ctx, type_cls);
    result.this_ = tvAsVariant(ctx).toObject().get();
  } else {
    FTRACE(1, "Unhandled context type {}: {}\n",
           func->fullName()->data(), show(ctx));
    return std::nullopt;
  }

  auto const method = func->implCls();
  auto const static_method = method && func->isStaticInPrologue();
  auto const instance_method = method && !static_method;

  if (instance_method && (!result.this_ || !result.this_->instanceof(cls))) {
    FTRACE(1, "Instance method context mismatch {}: {}\n",
           func->fullName()->data(), show(ctx));
    return std::nullopt;
  } else if (static_method && (!result.cls || !result.cls->subtypeOf(cls))) {
    FTRACE(1, "Static method context mismatch {}: {}\n",
           func->fullName()->data(), show(ctx));
    return std::nullopt;
  } else if (!method && (result.this_ || result.cls)) {
    FTRACE(1, "Static method context mismatch {}: {}\n",
           func->fullName()->data(), show(ctx));
    return std::nullopt;
  }
  return result;
}

}

//////////////////////////////////////////////////////////////////////////////

InlineInterpHookResult callInlineInterpHook() {
  using IIHR = InlineInterpHookResult;
  auto const sk = vmsk();
  FTRACE(2, "  {}: {}\n", showShort(sk), sk.showInst());
  if (sk.prologue()) return IIHR::NONE;

  rl_context->numOps++;
  if (auto const ok = tryCustomInlineInterpStep(sk)) return *ok;
  return handledByInterpreter(sk) ? IIHR::NONE : IIHR::STOP;
}

Optional<ROMHandle> runInlineInterp(const Func* func, TypedValue context,
                                    uint32_t nargs, const TypedValue* args,
                                    RuntimeCoeffects coeffects,
                                    uint64_t* numOps) {
  using IIS = ExecutionContext::InlineInterpState;
  always_assert(g_context->m_inlineInterpState == IIS::NONE);
  g_context->m_inlineInterpState = IIS::START;
  *rl_context = InlineInterpContext{};

  ProfileNonVMThread _;
  auto const forceInterpret = rl_typeProfileLocals->forceInterpret;
  rl_typeProfileLocals->forceInterpret = true;
  RID().updateJit();
  assertx(!RID().getJit());

  SCOPE_EXIT {
    FTRACE(1, "runInlineInterp finished/failed after {} ops\n",
           rl_context->numOps);
    *rl_context = InlineInterpContext{};
    g_context->m_inlineInterpState = IIS::NONE;
    rl_typeProfileLocals->forceInterpret = forceInterpret;
    RID().updateJit();
  };

  // Validate the arguments and turn anonymous MysteryBox args into ones that
  // track their known local provenance. TODO(kshaunak): We should convert all
  // non-MysteryBox args into static values, and fail if that's not possible.
  FTRACE(1, "runInlineInterp: {}\n", func->fullName());
  auto const ctx = resolveCallContext(func, context);
  if (!ctx) return std::nullopt;

  auto args_copy = std::vector<TypedValue>(nargs);
  for (auto i = uint32_t{0}; i < nargs; i++) {
    args_copy[i] = args[i];
    if (!materializeBox(&args_copy[i], {MysteryBoxType::Local, i})) {
      FTRACE(1, "Failed to materialize argument {}: {}\n\n", i, show(args[i]));
      return std::nullopt;
    }
    FTRACE(2, "  Argument {}: {}\n", i, show(args_copy[i]));
  }

  try {
    ThrowAllErrorsSetter taes;
    FTRACE(1, "runInlineInterp: {} started:\n", func->fullName());
    auto const result = g_context->invokeFuncFew(
        *ctx, nargs, &args_copy[0], coeffects);
    FTRACE(1, "runInlineInterp: {} succeeded!\n", func->fullName());
    if (numOps) *numOps = rl_context->numOps;
    std::vector<const MysteryBoxData*> boxes;
    for (auto const& box : rl_context->boxes) {
      boxes.push_back(box->data);
      auto const& cs = box->data->constraints;
      DEBUG_ONLY auto const tv = make_tv<KindOfResource>(box->hdr());
      FTRACE(1, "  Found {} constraints for: {}\n", cs.size(), show(tv));
      for (UNUSED auto const& c : cs) {
        FTRACE(1, "    {} (ctx: {}, prop decl: {})\n",
               c.tc.displayName(),
               c.ctx ? c.ctx->name()->data() : "null",
               c.propDecl ? c.propDecl->name()->data() : "not prop");
      }
    }
    FTRACE(1, "\n");
    ROMHandle rom(std::make_unique<ROMData>(std::vector<TypedValue>{result}, boxes));
    FTRACE(1, "ROM: \n{}", rom.data->toString());
    return Optional<ROMHandle>(std::move(rom));
  } catch (const Object& obj) {
    FTRACE(1, "Caught Hack exception: {}\n", obj->getVMClass()->name());
  } catch (const std::exception& exn) {
    FTRACE(1, "Caught C++ exception: {}\n", exn.what());
  } catch (...) {
    always_assert_flog(0, "runInlineInterp caught a non-std::exception");
  }
  return std::nullopt;
}

//////////////////////////////////////////////////////////////////////////////

}
