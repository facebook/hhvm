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

using LowStrPtr = LowPtr<const StringData>;

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

// Other values

void takeStringData(StringData* UNUSED v) { return; }
void takeString(String UNUSED v) { return; }
void takeStaticString(StaticString UNUSED v) { return; }
void takeStrNR(StrNR UNUSED v) { return; }
void takeResource(Resource UNUSED v) { return; }
void takeObject(Object UNUSED v) { return; }
void takeReqPtr(req::ptr<ObjectData> UNUSED v) { return; }
void takeOptional(Optional<String> UNUSED v) { return; }
void takeLowPtr(LowPtr<Class> UNUSED v) { return; }
void takeLowStrPtr(LowStrPtr UNUSED v) { return; }
void takeExtension(Extension UNUSED v) { return; }
void takeArrayData(ArrayData UNUSED *v) { return; }
void takeArrayVec(Array UNUSED v) { return; }
void takeArrayDict(Array UNUSED v) { return; }
void takeArrayKeyset(Array UNUSED v) { return; }

void buildOtherValues() {
  TestObject = SystemLib::AllocInvalidArgumentExceptionObject("This is a test exception object for lldb");
  Array vec = make_vec_array(1, 2, 3, 4);
  Array dict = make_dict_array(0x0123cafe, true, 302, "Salutations, earth!", 2, 3.14, "key4", 2.718, "key5", "Hello, world!");
  Array keyset = make_keyset_array(1, "cats", 2, 3, "cats", 2, 3, "dogs", 42);

  takeStringData(StringData::MakeStatic("hello"));
  takeString(String("hello"));
  takeStaticString(StaticString("hello"));
  takeStrNR(StrNR(StringData::MakeStatic("hello")));
  takeResource(Resource(req::make<DummyResource>()));
  takeObject(TestObject);
  takeReqPtr(*reinterpret_cast<req::ptr<ObjectData> *>(&TestObject)); // Want to get its sole private member m_obj
  takeOptional(Optional<String>("hello"));
  takeOptional(Optional<String>());
  takeLowPtr(LowPtr(TestObject->getVMClass()));
  takeLowStrPtr(LowStrPtr(StringData::MakeStatic("hello")));
  takeExtension(Extension("test-extension", "0.5", "test-oncall"));
  takeArrayData(vec.get());
  takeArrayVec(vec);
  takeArrayDict(dict);
  takeArrayKeyset(keyset);
}

} // namespace lldb_test
} // namespace HPHP

int main(int argc, char** argv) {
  HPHP::rds::local::init();
  SCOPE_EXIT { HPHP::rds::local::fini(); };
  HPHP::init_for_unit_test();
  SCOPE_EXIT { HPHP::hphp_process_exit(); };

  if (argc < 2) {
    std::cout << "Specify what to run (options: \"typed-values\", \"other-values\")" << std::endl;
    return 1;
  }
  if (!strcmp(argv[1], "typed-values")) {
    HPHP::lldb_test::buildTypedValues();
  } else if (!strcmp(argv[1], "other-values")) {
    HPHP::lldb_test::buildOtherValues();
  } else {
    std::cout << "Invalid option (options: \"typed-values\", \"other-values\"" << std::endl;
    return 1;
  }
  return 0;
}
