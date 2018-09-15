<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/*
 * The following enum values are defined in
 * hphp/runtime/ext/reflection/ext_reflection-classes.php
 */
enum TypeStructureKind : int {
  OF_VOID = 0;
  OF_INT = 0;
  OF_BOOL = 0;
  OF_FLOAT = 0;
  OF_STRING = 0;
  OF_RESOURCE = 0;
  OF_NUM = 0;
  OF_ARRAYKEY = 0;
  OF_NORETURN = 0;
  OF_MIXED = 0;
  OF_TUPLE = 0;
  OF_FUNCTION = 0;
  OF_ARRAY = 0;
  OF_GENERIC = 0;
  OF_SHAPE = 0;
  OF_CLASS = 0;
  OF_INTERFACE = 0;
  OF_TRAIT = 0;
  OF_ENUM = 0;
  OF_DICT = 0;
  OF_VEC = 0;
  OF_KEYSET = 0;
  OF_VEC_OR_DICT = 0;
  OF_NONNULL = 0;
  OF_DARRAY = 0;
  OF_VARRAY = 0;
  OF_VARRAY_OR_DARRAY = 0;
  OF_UNRESOLVED = 0;
}

// Note: Nullable fields in shapes of this type may not be present, and so
// should be considered optional. Additionally, shapes of this type may contain
// additional fields other than those specified here.
newtype TypeStructure<T> as shape(
  'nullable' => ?bool,
  'kind' => TypeStructureKind,
  'name' => ?string,
  'classname' => ?classname<T>,
  /* HH_IGNORE_ERROR[2071] */
  'elem_types' => ?varray,
  /* HH_IGNORE_ERROR[2071] */
  'return_type' => ?darray,
  /* HH_IGNORE_ERROR[2071] */
  'param_types' => ?varray,
  /* HH_IGNORE_ERROR[2071] */
  'generic_types' => ?varray,
  'root_name' => ?string,
  /* HH_IGNORE_ERROR[2071] */
  'access_list' => ?varray,
  /* HH_IGNORE_ERROR[2071] */
  'fields' => ?darray,
  'allows_unknown_fields' => ?bool,
  'is_cls_cns' => ?bool,
  'optional_shape_field' => ?bool,
  /* HH_IGNORE_ERROR[2071] */
  'value' => ?darray,
  'typevars' => ?string,
  'alias' => ?string,
) = shape(
  'nullable' => ?bool,
  'kind' => TypeStructureKind,
  // name for generics (type variables)
  'name' => ?string,
  // classname for classes, interfaces, enums, or traits
  'classname' => ?classname<T>,
  // for tuples
  /* HH_IGNORE_ERROR[2071] */
  'elem_types' => ?varray,
  /* HH_IGNORE_ERROR[2071] */
  'return_type' => ?darray,
  // for functions
  /* HH_IGNORE_ERROR[2071] */
  'param_types' => ?varray,
  // for arrays, classes
  /* HH_IGNORE_ERROR[2071] */
  'generic_types' => ?varray,
  'root_name' => ?string,
  /* HH_IGNORE_ERROR[2071] */
  'access_list' => ?varray,
  // for shapes
  /* HH_IGNORE_ERROR[2071] */
  'fields' => ?darray,
  'allows_unknown_fields' => ?bool,
  'is_cls_cns' => ?bool,
  'optional_shape_field' => ?bool,
  /* HH_IGNORE_ERROR[2071] */
  'value' => ?darray,
  // Comma-separated string
  'typevars' => ?string,
  // for type aliases
  'alias' => ?string,
);

/*
 * returns the shape associated with the type constant.
 */
<<__Rx>>
function type_structure(mixed $cls_or_obj, string $cns_name);
// becomes:
// type_structure(C::class or new C, 'type_const_name')
//   : TypeStructure
