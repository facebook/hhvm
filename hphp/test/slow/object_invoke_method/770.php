<?php

// with var args
class C9 {
  public function __invoke() {
    var_dump(func_num_args());
    var_dump(func_get_args());
  }
}

<<__EntryPoint>>
function main_770() {
$c = new C9;
$c();
$c(0);
$c(0, 1);
}
