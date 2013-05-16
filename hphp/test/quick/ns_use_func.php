<?php
namespace A {
  function f() { return 'a'; }
}
namespace B {
  use A\f;
  var_dump(f());
}
