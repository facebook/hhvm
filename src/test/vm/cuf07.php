<?php
function foo() { return "hello"; }
function bar() { return "goodbye"; }
function test() {
  var_dump(fb_call_user_func_safe_return('foo', 0));
}
function main() {
  test();
  fb_rename_function('foo', 'baz');
  test();
  fb_rename_function('bar', 'foo');
  test();
}
main();
