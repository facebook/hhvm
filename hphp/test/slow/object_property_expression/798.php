<?hh

class Y {
}
class X {
  public $a;
  function __construct() {
    $this->a = darray['x' => new Y];
  }
  function bar() {
    var_dump('bar');
    $this->qq = new Y;
    $this->qq->x = $this->qq->y = 1;
    return $this->qq;
  }
}
function foo() {
 var_dump('foo');
 return 'foo';
}
function test($x, $a, $s) {
  unset($x->bar()->x);
  unset($x->q->r->s->$foo);
  unset($x->y->a->b->c);
  unset($x->a['x']->y->a->b->c);
  unset($a['a']['y'][foo()]);
  unset($a['b']->y->z);
  unset($a->c->d);
  var_dump($x, $a, $s);
}

<<__EntryPoint>>
function main_798() {
  $s = false;
  test(new X, varray[], $s);
}
