<?hh

function foo() { var_dump('failed'); }

fb_intercept('foo', 'bar', "hello");

function bar() {
  var_dump(func_get_args());
  return 25;
}

function main() {
  var_dump(foo(1,2));
}

main();

