<?hh

type Validator = (function(mixed): bool);

function foo(Validator $func) {
  var_dump($func("yo"));
}

function asd(mixed $k): bool { return true; }


<<__EntryPoint>>
function main_typedef_closure() {
foo(fun('asd'));
}
