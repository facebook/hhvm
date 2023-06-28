<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo {
  public $name;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(unserialize("O:3:\"Foo\":1:{S:\x00\x00\x00\x00\x00\x00\x00\x00;s:5:\"value\";}"));
}
