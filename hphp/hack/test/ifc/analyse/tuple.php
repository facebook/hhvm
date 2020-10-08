<?hh

class C {}

function triple(int $i, C $c): (int,string,C) {
  return tuple($i, "hello", $c);
}

function nested(int $i, C $c): (int,(string,C)) {
  return tuple($i, tuple("hello", $c));
}
