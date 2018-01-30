<?hh // strict

function test((string, int) $tup, Traversable<int> $ints, string ...$y): void {
  // OK
  $lambda = (...$x) ==> $x[0] + $x[1];
  $lambda(...$ints);

  // Error, $single_int is not unpackable
  $single_int = 1;
  $lambda = (...$x) ==> $x[0] + $x[1];
  $lambda(...$single_int);

  // Error, expecting string but got int
  $lambda = ($a, string ...$x) ==> $x[0]."hello";
  $lambda(1, 1, ...$y);

  // OK, as second argument is a string
  $lambda = ($a, ...$x) ==> $x[0]."hello";
  $lambda(1, "hello", ...$y);

  // OK
  $lambda = (...$x) ==> $x[0]."hello";
  $lambda(...$y);

  // OK
  $lambda = (string ...$x) ==> $x[0]."hello";
  $lambda(...$y);

  // OK
  $lambda = (...$x) ==> $x[0]."hello";
  $lambda(...$ints);

  // OK
  $lambda = ($a, int ...$x) ==> $x[0]."hello";
  $lambda("some param", ...$ints);

  // Error, expecting variadic string, we pass it a varray
  $lambda = (...$x) ==> $x[0].$x[1];
  $lambda($y);

  // Error, $x will be variadic string
  $lambda = (...$x) ==> $x[0]->foo();
  $lambda(...$y);

  // Error, $b will be an int when $tup is unpacked into $lambda.
  $lambda = ($a, $b) ==> {
    return $b->foo();
  };
  $lambda(...$tup);

  // OK
  $lambda = ($a, $b) ==> {
    return $b + 1;
  };
  $lambda(...$tup);
}
