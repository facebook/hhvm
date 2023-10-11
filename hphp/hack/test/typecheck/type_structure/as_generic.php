<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T>(TypeStructure<T> $ts): void {
  hh_show($ts['kind']);
  hh_show(Shapes::idx($ts, 'alias'));
  hh_show(Shapes::idx($ts, 'name'));
  hh_show(Shapes::idx($ts, 'classname'));
  hh_show(Shapes::idx($ts, 'param_types'));
  hh_show(Shapes::idx($ts, 'return_type'));
  hh_show(Shapes::idx($ts, 'nullable'));
  hh_show(Shapes::idx($ts, 'elem_types'));
  hh_show(Shapes::idx($ts, 'fields'));
  hh_show(Shapes::idx($ts, 'generic_types'));
}
