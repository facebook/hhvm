<?php

print "Test begin\n";

class E implements ArrayAccess {
  public function __destruct() {
    print "In E::__destruct()\n";
  }
  public function offsetGet($offset) {
    print "In E::offsetGet()\n";
    $a = array();
    $a[] = "hello";
    $a[] = new E;  // to make sure the array's life cycle is correct
    return $a;
  }
  public function offsetExists($offset) {}
  public function offsetSet($offset, $value) {}
  public function offsetUnset($offset) {}
}

class D implements ArrayAccess {
  public function __destruct() {
    print "In D::__destruct()\n";
  }
  public function offsetGet($offset) {
    print "In D::offsetGet()\n";
    # Generate a new object that has no references besides the one being
    # returned, and return it.
    return new E;
  }
  public function offsetExists($offset) {}
  public function offsetSet($offset, $value) {}
  public function offsetUnset($offset) {}
}

class C implements ArrayAccess {
  public function offsetGet($offset) {
    print "In C::offsetGet()\n";
    # Generate a new object that has no references besides the one being
    # returned, and return it.
    return new D;
  }
  public function offsetExists($offset) {}
  public function offsetSet($offset, $value) {}
  public function offsetUnset($offset) {}
}

function main1() {
  $c = new C;
# Dereference the object returned by $c['x'], all in a single expression.  The
# VM must retain a reference to the intermediate result for long enough to
# complete the next dereference operation.
  $x = $c['d']['e'][0][0];
  var_dump($x);
  $c['x']['y'][0][0] = "goodbye";
}

main1();

class cls implements arrayaccess {
  private $container = array();
  public function __construct() {
    $this->container = array(
                             "one"   => 1,
                             "two"   => 2,
                             "three" => 3,
                            );
  }
  public function offsetGet($offset) {
    print "In cls::offsetGet($offset)\n";
    return isset($this->container[$offset]) ? $this->container[$offset] : null;
  }
  public function offsetExists($offset) {
    print "In cls::offsetExists($offset)\n";
    return isset($this->container[$offset]);
  }
  public function offsetSet($offset, $value) {
    print "In cls::offsetSet($offset, $value)\n";
    if (is_null($offset)) {
      $this->container[] = $value;
    } else {
      $this->container[$offset] = $value;
    }
  }
  public function offsetUnset($offset) {
    print "In cls::offsetUnset($offset)\n";
    unset($this->container[$offset]);
  }
}

function main2() {
  $obj = new cls;

  var_dump(isset($obj["two"]));
  var_dump($obj["two"]);
  unset($obj["two"]);
  $a = array($obj);
  $obj["two"] = 2;
  unset($a[0]["two"]);
  var_dump(isset($obj["two"]));
  $obj["two"] = "A value";
  var_dump($obj["two"]);
  $obj[] = 'Append 1';
  $obj[] = 'Append 2';
  $obj[] = 'Append 3';
  $obj[][0] = "lost"; # offsetGet() returns null by value, so array("lost") is
  # lost.

  # SetOpElem.
  $obj["x"] += 1;
  var_dump($obj["x"]);
  $obj["x"] -= 1;
  var_dump($obj["x"]);

  # IncDecElem.
  $obj["x"]++;
  var_dump($obj["x"]);
  $obj["x"]--;
  var_dump($obj["x"]);
  ++$obj["x"];
  var_dump($obj["x"]);
  --$obj["x"];
  var_dump($obj["x"]);

  $obj[] = "SetNewElem";
  $obj[] += 1; # SetOpNewElem.
  $obj[]++; # IncDecNewElem.

  # UnsetElem.
  print_r($obj);
  unset($obj["x"]);
  print_r($obj);

  print "Test end\n";
}
main2();

class stringdoubler implements ArrayAccess {
  public function offsetExists($i) {
    return is_string($i);
  }
  public function offsetGet($i) {
    return $i . $i;
  }
  public function offsetSet($i, $v) {}
  public function offsetUnset($i) {}
}

function main3($a, $b, $c) {
  if (false) {}
  return $a[$b][$c];
}
var_dump(main3(array(new stringdoubler()), 0, 'hello'));
