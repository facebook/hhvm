<?hh

function foo(int $x, string $y = 'foo') :mixed{
  var_dump($y);
}

<<__EntryPoint>>
function main() :mixed{
  foo(42);
}
