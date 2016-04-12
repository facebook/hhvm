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

function f5(): Vector<(int, string,)> {
  return Vector {tuple(42, "foobar")};
}

function f6(): Vector<(int, string)> {
  return Vector {tuple(42, "foobar")};
}

function f7(): Vector<(function(int,): int)> {
  return Vector {(int $a) ==> $a};
}

function f8(): Vector<(function(int): int)> {
  return Vector {(int $a) ==> $a};
}

echo 'Done';
