<?hh

class C {}

<<__InferFlows>>
function triple(int $i, C $c): (int,string,C) {
  return tuple($i, "hello", $c);
}

<<__InferFlows>>
function nested(int $i, C $c): (int,(string,C)) {
  return tuple($i, tuple("hello", $c));
}
