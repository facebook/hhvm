<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  var_dump(SOME_CONST);
  var_dump(some_func());
  $x = new SomeClass();
  var_dump($x->foo());
}
