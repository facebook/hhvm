<?hh

function foo(int $x, string $y = 'foo') {
  var_dump($y);
}

<<__EntryPoint>>
function main() {
  foo(42);
}
