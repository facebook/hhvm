<?hh
function foo() :mixed{ return "hello"; }
function bar() :mixed{ return "goodbye"; }
function test() :mixed{
  if (function_exists('foo')) {
    var_dump(foo());
  } else {
    var_dump(0);
  }
}
<<__EntryPoint>> function main(): void {
  test();
  fb_rename_function('foo', 'baz');
  test();
  fb_rename_function('bar', 'foo');
  test();
}
