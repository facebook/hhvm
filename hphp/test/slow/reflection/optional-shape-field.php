<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func(): shape(?'key1' => ?bool,
                       'key2'  => int,
                       ?'key3' => string,
                       'key4'  => ?int) {
  return shape();
}


<<__EntryPoint>>
function main_optional_shape_field() :mixed{
var_dump((new ReflectionFunction('func'))->getReturnTypeText());
}
