<?php /* destructor */

class C {
  private $a;
  function __construct(&$a) { $this->a =& $a; }
  function __destruct() {
    // tripling the number of things in an array should cause it to realloc
    $c = count($this->a) * 2;
    for ($i = 0; $i < $c; $i++) { $this->a[] = 'lol'; }
  }
}

function test() {
  $a = array(1, 2, 3);
  $a[] = new C($a);
  array_splice($a, 2);
  var_dump($a);
}


<<__EntryPoint>>
function main_array_splice_destructors() {
test();
}
