<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  const string FOO = 'foo';
}

function foo() {
  Foo::FOO;
}

const string BAR = Foo::FOO;
