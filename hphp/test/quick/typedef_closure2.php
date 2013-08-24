<?hh

type OptValidator = ?(function(mixed): bool);

function foo(OptValidator $func) {
  if ($func) {
    var_dump($func("yo"));
  }
}

function asd(mixed $k): bool { return true; }

foo('asd');
foo(null);
