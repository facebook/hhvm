<?php
namespace A {
  const C = "a";
}
namespace B {
  use A\C;
  var_dump(C);
}
