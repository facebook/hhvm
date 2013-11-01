<?php
function add($arg1,$arg2) {
  if ($arg1 == 0) {
    return "Fluffy";
  }
  return $arg1 + $arg2;
}


class Foo {
  static function s_identity($param){
    if ($param == 0) {
      return "String";
    }
    return $param;
  }
  function identity($param) {
    if ($param == 0) {
      return "String";
    }
    return $param;
  }
}

function main() {
  $a = new Foo();
  $a->identity(232);
  $a->identity(42);
  $a->identity(0);
  add(1,1);
  add(0,0);
}

main();
?>

