<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() {
  var_dump('foo');
}

function bar() {
  var_dump('bar');
}
<<__EntryPoint>> function main(): void {
register_postsend_function('bar');
register_shutdown_function(fun('foo'));
}
