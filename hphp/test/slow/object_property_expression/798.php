<?hh

class Y {
}
class X {
  public $a;
  function __construct() {
    $this->a = dict['x' => new Y];
  }
  function bar() :mixed{
    var_dump('bar');
    $this->qq = new Y;
    $this->qq->x = $this->qq->y = 1;
    return $this->qq;
  }
}
function foo() :mixed{
  var_dump('foo');
  return 'foo';
}
function test($x, $a, $s) :mixed{
  unset($x->bar()->x);
  try {
    unset($x->q->r->s->$foo);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  unset($x->y->a->b->c);
  unset($x->a['x']->y->a->b->c);
  unset($a['a']['y'][foo()]);
  unset($a['b']->y->z);
  unset($a->c->d);
  var_dump($x, $a, $s);
}

<<__EntryPoint>>
function main_798() :mixed{
  $s = false;
  test(new X, vec[], $s);
}
