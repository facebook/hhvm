<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T>(TypeStructure<T> $ts): void {
  hh_show($ts['kind']);
  hh_show($ts['alias']);
  hh_show($ts['name']);
  hh_show($ts['classname']);
  hh_show($ts['param_types']);
  hh_show($ts['return_type']);
  hh_show($ts['nullable']);
  hh_show($ts['elem_types']);
  hh_show($ts['fields']);
  hh_show($ts['generic_types']);
}
