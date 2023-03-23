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

#include "hphp/hhvm/process-init.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {
namespace lldb_test {

Object TestObject;

template <DataType DT>
TypedValue createTestTypedValue() {
  switch (DT) {
    case DataType::PersistentDict:
      return make_tv<DataType::PersistentDict>(staticEmptyDictArray());
    case DataType::Dict: {
      Array d = make_dict_array(
        "key1", 1,
        "key2", 2.718,
        "key3", "Hello, world!"
      );
      return make_tv<DataType::Dict>(d.get());
    }
    case DataType::PersistentVec:
      return make_tv<DataType::PersistentVec>(staticEmptyVec());
    case DataType::Vec: {
      Array v = make_vec_array(1, 2, 3);
      return make_tv<DataType::Vec>(v.get());
      break;
    }
    case DataType::PersistentKeyset:
      return make_tv<DataType::PersistentKeyset>(staticEmptyKeysetArray());
    case DataType::Keyset: {
      Array k = make_keyset_array(1, 2, 3);
      return make_tv<DataType::Keyset>(k.get());
    }
    case DataType::PersistentString:
      return make_tv<DataType::PersistentString>(StringData::MakeStatic("Hello, world!"));
    case DataType::String:
      return make_tv<DataType::String>(StringData::Make("Hello, world!"));
    case DataType::Object:
      return make_tv<DataType::Object>(TestObject.get());
    case DataType::Resource: {
      auto rsc = req::make<DummyResource>();
      return make_tv<DataType::Resource>(rsc->hdr());
    }
    // Note: the data used to build RFunc and RClsMeth here are totally contrived
    // (i.e. the function used here really doesn't take reified generics).
    case DataType::RFunc: {
      auto func = TestObject->getVMClass()->getCtor();
      auto arr = staticEmptyVec();
      auto rfuncdata = RFuncData::newInstance(const_cast<Func *>(func), arr);
      return make_tv<KindOfRFunc>(rfuncdata);
    }
    case DataType::RClsMeth: {
      auto cls = TestObject->getVMClass();
      auto func = cls->getCtor();
      auto arr = staticEmptyVec();
      auto rclsmethdata = RClsMethData::create(cls, const_cast<Func *>(func), arr);
      return make_tv<KindOfRClsMeth>(rclsmethdata);
    }
    case DataType::ClsMeth: {
      auto cls = TestObject->getVMClass();
      auto func = cls->getCtor();
      auto clsmethdata = ClsMethDataRef::create(cls, const_cast<Func*>(func));
      return make_tv<KindOfClsMeth>(clsmethdata);
    }
    case DataType::Boolean:
      return make_tv<DataType::Boolean>(true);
    case DataType::Int64:
      return make_tv<DataType::Int64>(42);
    case DataType::Double:
      return make_tv<DataType::Double>(3.1415);
    case DataType::Func: {
      auto func = TestObject->getVMClass()->getCtor();
      return make_tv<KindOfFunc>(const_cast<Func *>(func));
    }
    case DataType::Class:
      return make_tv<KindOfClass>(TestObject->getVMClass());
    case DataType::LazyClass: {
      auto lazy_cls = LazyClassData::create(StringData::MakeStatic("SpecialLazyClass"));
      return make_tv<DataType::LazyClass>(lazy_cls);
    }
    case DataType::Uninit:
      return make_tv<KindOfUninit>();
    case DataType::Null:
      return make_tv<KindOfNull>();
  }
  not_reached();
}

#define DT(name, ...) \
  void takeTypedValue##name(TypedValue UNUSED tv) { return; }
DATATYPES
#undef DT

void takeTypedValueRef(TypedValue& UNUSED tv) { return; }

// TypedValue subtypes
void takeVariant(Variant UNUSED v) { return; }
void takeVarNR(VarNR UNUSED v) { return ; }

void buildTypedValues() {
  TestObject = SystemLib::AllocInvalidArgumentExceptionObject("This is a test exception object for lldb");

  #define DT(name, ...) \
  { \
      auto tv = createTestTypedValue<DataType::name>(); \
      takeTypedValue##name(std::move(tv)); \
  }
  DATATYPES
  #undef DT
  {
    auto tv = createTestTypedValue<DataType::Int64>();
    takeTypedValueRef(tv);
  }
  takeVariant(Variant(42));
  takeVarNR(VarNR(2.718));
}

void takeStringData(StringData* UNUSED s) { return; }

void buildStringData() {
  auto s = StringData::MakeStatic("hello");
  takeStringData(s);
}

} // namespace lldb_test
} // namespace HPHP

int main(int UNUSED argc, char** UNUSED argv) {
  HPHP::rds::local::init();
  SCOPE_EXIT { HPHP::rds::local::fini(); };
  HPHP::init_for_unit_test();
  SCOPE_EXIT { HPHP::hphp_process_exit(); };

  HPHP::lldb_test::buildTypedValues();
  HPHP::lldb_test::buildStringData();
}
