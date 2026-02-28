<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// interface FooBar {}

// class Foo implements FooBar {}
// class Bar implements FooBar {}

class Foo {}
newtype FooBar = Foo;

function dofoo(FooBar $f) :mixed{
  var_dump($f);
}

newtype StringAlias = string;
function dostring(StringAlias $s) :mixed{
  var_dump($f);
}
<<__EntryPoint>> function main(): void {
dofoo(new Foo());
dostring(new Foo());
}
