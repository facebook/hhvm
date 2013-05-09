<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

print "Test begin\n";

class A {
  static public $a = 1;
  static public function setA($val) {
    self::$a =& $val;
  }
}

function main(){
  echo "main\n";
  A::$a = 30;
  $x =& A::$a;
  print($x.A::$a."\n");
  $x = 5;
  print($x.A::$a."\n");
}

function main2($name){
  echo "main2\n";
  $name::$a = 30;
  $x =& $name::$a;
  print($x.$name::$a."\n");
  $x = 5;
  print($x.$name::$a."\n");
}

function main3() {
  A::setA(5);
}

main();
main();

main2("A");
main2("A");

main3();
main3();
