<?php

class A {
  public $a = 1;
  private $b = "hello";
  protected $c = array(1, 2);
}

class B extends A {
  public $b = 0;
}

class C {
  public $a, $b, $c;
  function __construct() {
    $this->a = null;
    $this->b = acos(1.01);
    $this->c = log(0);
    echo "C has a safe constructor.\n";
  }
  function __destruct() {
    echo "C has a safe destrcutor.\n";
  }
  function __wakeup() {
    echo "C wakes up safely.\n";
  }
  function __sleep() {
    echo "C sleeps safely.\n";
    return array('a', 'b', 'c');
  }
}

class DangerousClass {
  public $danger = "DangerousString";
  function __construct() {
    echo "I have dangerous constructor.\n";
  }
  function __destruct() {
    echo "I have dangerous destructor.\n";
  }
  function __wakeup() {
    echo "I wake up dangerously.\n";
  }
  function __sleep() {
    echo "I sleep dangerously.\n";
    return array('danger');
  }
}

class E {
  public $dangerousClass;
  function __construct() {
    $this->dangerousClass = new DangerousClass;
  }
}

class F implements Serializable {
  public function serialize() {
    return "SerializedData";
  }
  public function unserialize($serialized) {
    echo $serialized;
    return $this;
  }
}

class G extends DangerousClass {
}

function test_serialization($obj, $class_whitelist) {
  $str = serialize($obj);
  var_dump($str);
  $new_obj = unserialize($str, $class_whitelist);
  var_dump($new_obj);
  unset($obj);
  unset($new_obj);
  echo "========================\n";
}

function main(int $argc, array $argv) {
  // null will be autmatically translated into empty array (see idl definition)
  // So it should still not allow any class.
  test_serialization(new A, null);
  test_serialization(new B, array('A'));
  test_serialization(new C, array('C'));
  test_serialization(new DangerousClass, array());
  test_serialization(new E, array('E'));
  test_serialization(new F, array());
  test_serialization(new G, array('G'));
  test_serialization(array("Hello World<>$%", acos(1.01), log(0), 50), array());
  test_serialization(
    array( new A, array(new B, array(new C, array(new E, array(new F))))),
    array('abc' => 'A', 5 => 'C', 'E')
  );
}

exit(main($argc, $argv));
