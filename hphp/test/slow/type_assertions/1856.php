<?hh

class X {
  public $x = 'foo';
}
class Y {
  public $y = 'bar';
}
class X1 extends X {
  public $x = 'baz';
}
function f($x) :mixed{
  if ($x is X && isset($x->x)) {
    var_dump($x->x);
  }
  if ($x is Y && isset($x->y)) {
    var_dump($x->y);
  }
  if (is_a($x, 'X1')) {
    var_dump($x->x);
  }
}

<<__EntryPoint>>
function main_1856() :mixed{
f(null);
f(new X);
f(new Y);
f(new X1);
f(new stdClass());
}
