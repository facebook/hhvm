
/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT
 *  @generated
 *
 */

#pragma once

#include <thrift/lib/python/capi/constructor.h>
#include <thrift/lib/python/capi/extractor.h>

#include <thrift/compiler/test/fixtures/inject_metadata_fields/gen-cpp2/module_types.h>

namespace module {

struct NamespaceTag {};

} // namespace module

namespace apache::thrift::python::capi {
template <>
struct Extractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::Fields, ::module::NamespaceTag>>
    : public BaseExtractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::Fields, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  ExtractorResult<::cpp2::Fields> operator()(PyObject* obj);
  int typeCheck(PyObject* obj);
};

template <>
struct Extractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::Fields, ::module::NamespaceTag >>
    : public BaseExtractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::Fields, ::module::NamespaceTag>> {
  ExtractorResult<::cpp2::Fields> operator()(PyObject* obj);
};

template <>
struct Constructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::Fields, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::Fields, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  PyObject* operator()(const ::cpp2::Fields& val);
};

template <>
struct Constructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::Fields, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::Fields, ::module::NamespaceTag>> {
  PyObject* operator()(const ::cpp2::Fields& val);
};

template <>
struct Extractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>>
    : public BaseExtractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  ExtractorResult<::cpp2::FieldsInjectedToEmptyStruct> operator()(PyObject* obj);
  int typeCheck(PyObject* obj);
};

template <>
struct Extractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag >>
    : public BaseExtractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>> {
  ExtractorResult<::cpp2::FieldsInjectedToEmptyStruct> operator()(PyObject* obj);
};

template <>
struct Constructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  PyObject* operator()(const ::cpp2::FieldsInjectedToEmptyStruct& val);
};

template <>
struct Constructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToEmptyStruct, ::module::NamespaceTag>> {
  PyObject* operator()(const ::cpp2::FieldsInjectedToEmptyStruct& val);
};

template <>
struct Extractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>>
    : public BaseExtractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  ExtractorResult<::cpp2::FieldsInjectedToStruct> operator()(PyObject* obj);
  int typeCheck(PyObject* obj);
};

template <>
struct Extractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag >>
    : public BaseExtractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>> {
  ExtractorResult<::cpp2::FieldsInjectedToStruct> operator()(PyObject* obj);
};

template <>
struct Constructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  PyObject* operator()(const ::cpp2::FieldsInjectedToStruct& val);
};

template <>
struct Constructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedToStruct, ::module::NamespaceTag>> {
  PyObject* operator()(const ::cpp2::FieldsInjectedToStruct& val);
};

template <>
struct Extractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>>
    : public BaseExtractor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  ExtractorResult<::cpp2::FieldsInjectedWithIncludedStruct> operator()(PyObject* obj);
  int typeCheck(PyObject* obj);
};

template <>
struct Extractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag >>
    : public BaseExtractor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>> {
  ExtractorResult<::cpp2::FieldsInjectedWithIncludedStruct> operator()(PyObject* obj);
};

template <>
struct Constructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::PythonNamespaced<::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>> {
  static const bool kUsingMarshal = true;
  PyObject* operator()(const ::cpp2::FieldsInjectedWithIncludedStruct& val);
};

template <>
struct Constructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>>
    : public BaseConstructor<::apache::thrift::python::capi::ComposedStruct<
        ::cpp2::FieldsInjectedWithIncludedStruct, ::module::NamespaceTag>> {
  PyObject* operator()(const ::cpp2::FieldsInjectedWithIncludedStruct& val);
};

} // namespace apache::thrift::python::capi
