<?hh

function foo() { return '123'; }
function bar() { return 'hehehe'; }

<<__EntryPoint>> function main(): void {
  var_dump(foo());
  var_dump(bar());
  fb_rename_function('bar', 'baz');
  fb_rename_function('foo', 'bar');
  var_dump(bar());
}
