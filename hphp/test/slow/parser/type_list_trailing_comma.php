<?hh

function f1(): (int, string,) {
  return tuple(42, "foobar");
}

function f2(): (int, string) {
  return tuple(42, "foobar");
}

function f3(): (function(int,): int) {
  return (int $a) ==> $a;
}

function f4(): (function(int): int) {
  return (int $a) ==> $a;
}

echo 'Done';
