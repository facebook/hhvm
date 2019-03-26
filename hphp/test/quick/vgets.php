<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

print "Test begin\n";

class A {
  static public $a = 1;
}

function main(&$x){
  echo "main\n";
  A::$a = 30;
  print($x.A::$a."\n");
  $x = 5;
  print($x.A::$a."\n");
}

function main2($name, &$x){
  echo "main2\n";
  $name::$a = 30;
  print($x.$name::$a."\n");
  $x = 5;
  print($x.$name::$a."\n");
}

main(&A::$a);
main(&A::$a);

$name = "A";
main2("A", &$name::$a);
main2("A", &$name::$a);
