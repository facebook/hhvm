<?php

class A {
  function c() {
    return tuple(1,2);
  }
}

<<__EntryPoint>>
function main_tuple_non_hh() {
var_dump((new A)->c());
}
