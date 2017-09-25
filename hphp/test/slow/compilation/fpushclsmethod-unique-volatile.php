<?hh

if (!apc_fetch('foo')) {
  apc_store('foo', 1);
  class X {
    static function foo($i) {
      $t = new static;
      $t->bar($i);
    }
    function bar($i) { if (!$i) var_dump('here'); }
  }
  class Y extends X {}
} else {
  apc_store('foo', 0);
}

function main($i) {
  X::foo($i);
}

for ($i = 0; $i < 100; $i++) main($i);
