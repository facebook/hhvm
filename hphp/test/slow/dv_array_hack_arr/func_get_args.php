<?hh

function bar(varray<mixed> $args): void {
  var_dump($args);
}

function foo(...$_args): void {
  bar(func_get_args());
}


<<__EntryPoint>>
function main_func_get_args() {
foo();
foo(1, 2, 3);
}
