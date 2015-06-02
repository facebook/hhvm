<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// interface FooBar {}

// class Foo implements FooBar {}
// class Bar implements FooBar {}

class Foo {}
newtype FooBar = Foo;

function dofoo(FooBar $f) {
  var_dump($f);
}

newtype StringAlias = string;
function dostring(StringAlias $s) {
  var_dump($f);
}

dofoo(new Foo());
dostring(new Foo());
