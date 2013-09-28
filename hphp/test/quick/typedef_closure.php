<?hh

type Validator = (function(mixed): bool);

function foo(Validator $func) {
  var_dump($func("yo"));
}

function asd(mixed $k): bool { return true; }

foo('asd');
