<?hh //partial

function foo(int $x) : void {
  function (int $x) { return $x + 1; };
}
