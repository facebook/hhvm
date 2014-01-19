<?php

class A {
}
class B extends A {
  function meh() {
    return $this;
  }
}
class C extends B {
  function work() {
    echo "WORK
";
  }
}
if (false) {
  class A {
}
  class B {
}
  class C {
}
}
function test() {
  $x = new C;
  $x->meh()->work();
}
test();
