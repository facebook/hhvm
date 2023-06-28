<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() :mixed{
  var_dump('foo');
}

function bar() :mixed{
  var_dump('bar');
}
<<__EntryPoint>> function main(): void {
register_postsend_function(bar<>);
register_shutdown_function(foo<>);
}
