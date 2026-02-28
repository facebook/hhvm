<?hh

type OptValidator = ?(function(mixed): bool);

function foo(OptValidator $func) :mixed{
  if ($func) {
    var_dump($func("yo"));
  }
}

function asd(mixed $k): bool { return true; }


<<__EntryPoint>>
function main_typedef_closure2() :mixed{
foo(asd<>);
foo(null);
}
