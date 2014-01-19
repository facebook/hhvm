<?php

class A_DateTime extends DateTime {
  public $___x;
  public function __clone() {
    $this->___x++;
  }
}

function test($cls, $args = array()) {
  echo $cls . "\n";
  $a = (new ReflectionClass($cls))->newInstanceArgs($args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && (get_class($c) != "__PHP_Unserializable_Class")) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));

  echo "================\n";

  $cls = 'A_' . $cls;
  echo $cls . "\n";
  $a = (new ReflectionClass($cls))->newInstanceArgs($args);
  var_dump($a);
  // serialize and unserialize
  $b = serialize($a);
  var_dump($b);
  $c = unserialize($b);
  var_dump($c);
  if (($a != $c) && (get_class($c) != "__PHP_Unserializable_Class")) {
    echo "bad serialization/deserialization\n";
    exit(1);
  }
  // get class methods
  var_dump(get_class_methods($a));
}

test("DateTime", array("2012-06-23T11:00:00"));

function main() {
  echo "================\n";

  $y = new A_DateTime("2012-06-23T11:00:00");
  $y->___y = 73;
  $y2 = clone $y;
  $y2->___y++;
  $y2->modify("+3 days");

  var_dump($y);
  var_dump($y->format('Y-m-d'));
  var_dump($y2);
  var_dump($y2->format('Y-m-d'));
}
main();
