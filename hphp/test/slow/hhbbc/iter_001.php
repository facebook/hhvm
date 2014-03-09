<?php

class C { function heh() { echo "hi\n"; } }
function foo() {
  $x = array(new C, new C);
  foreach ($x as $v) {
    $v->heh();
  }
}
foo();
