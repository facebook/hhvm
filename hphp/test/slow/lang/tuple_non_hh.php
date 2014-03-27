<?php

class A {
  function c() {
    return tuple(1,2);
  }
}
var_dump((new A)->c());
