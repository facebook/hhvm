<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
 * The following enum values are defined in
 * hphp/runtime/ext/reflection/ext_reflection-classes.php
 */

enum TypeStructureKind: int {
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
}

/*
 * Exposing the TypeStructure shapes to users. To avoid cyclic definition of
 * shapes, we define UnknownTypeStructure to be the subtype.
 */
newtype UnknownTypeStructure = shape(
  'nullable' => ?bool,
  'kind' => TypeStructureKind,
);

newtype TypeStructure = shape(
  'nullable' => ?bool,
  'kind' => TypeStructureKind,

  // for tuples
  'elem_types' => ?array<UnknownTypeStructure>,
  // for functions
  'param_types' => ?array<UnknownTypeStructure>,
  'return_type' => ?UnknownTypeStructure,
  // for arrays, classes
  'generic_types' => ?array<UnknownTypeStructure>,
  // for shapes
  'fields' => ?array<arraykey, UnknownTypeStructure>,
  // classname for classes
  'classname' => ?string,
  // name for generics (type variables)
  'name' => ?string,
);
