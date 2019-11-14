<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(
    dict[
      'class' => dict['someclass' => 'symbol_refs_class.inc'],
      'constant' => dict['SOME_CONST' => 'symbol_refs_const.inc'],
      'function' => dict['some_func' => 'symbol_refs_func.inc'],
    ],
    __DIR__.'/',
  );

  var_dump(SOME_CONST);
  var_dump(some_func());
  $x = new SomeClass();
  var_dump($x->foo());
}
