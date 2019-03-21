<?php

Function test_array_list() {
  List ($x, $y, $z) = Array (1, 2, 3);
  var_dump($x, $y, $z);
}

function test_closure_use() {
  $i = 0;
  $f = Function() Use ($i) {
    ECHO "CLOSURE $i\n";
    $i++;
  };
  $f();
  $f();
}

Function test_callable(Callable $c) {
  $c();
}


<<__EntryPoint>>
function main_misc() {
test_array_list();
test_closure_use();
test_callable(Function() {
  ECHO "CALL ME DEFINITELY\n";
});
}
