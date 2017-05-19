<?hh

class C {
  function __construct(private $v) {}
  function __destruct() {
    // tripling the number of things in a Vector should cause it to realloc
    $c = $this->v->count() * 2;
    for ($i = 0; $i < $c; $i++) { $this->v[] = 'lol'; }
  }
}

class D {
  function __construct(private $v) {}
  function __destruct() {
    // clearing should replace the underlying ArrayData with an empty one
    $this->v->clear();
  }
}

function test() {
  $v = Vector {1, 2, 3};
  $v[] = new C($v);
  $v->resize(2, 'IGNOREME');
  var_dump($v);

  $v = Vector {1, 2, 3};
  $v[] = new D($v);
  $v->resize(2, 'IGNOREME');
  var_dump($v);
}

test();
