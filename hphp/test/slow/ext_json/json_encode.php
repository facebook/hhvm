<?php

var_dump(json_encode(array("a" => 1, "b" => 2.3, 3 => "test")));
var_dump(json_encode(array("a", 1, true, false, null)));

var_dump(json_encode("a\xE0"));
var_dump(json_encode("a\xE0", JSON_FB_LOOSE));

var_dump(json_encode(array("0" => "apple", "1" => "banana")));

var_dump(json_encode(array(array("a" => "apple"))));

var_dump(json_encode(array(array("a" => "apple")), JSON_PRETTY_PRINT));

var_dump(json_encode(array(1, 2, 3, array(1, 2, 3)), JSON_PRETTY_PRINT));

$arr = array(
  "a" => 1,
  "b" => array(1, 2),
  "c" => array("d" => 42)
);
var_dump(json_encode($arr, JSON_PRETTY_PRINT));

class SerializableObject implements JsonSerializable {

  public function jsonSerialize() {
    return array('foo' => 'bar');
  }

}
var_dump(json_encode(new SerializableObject()));

class MultipleNonCircularReference implements JsonSerializable {

  public function jsonSerialize() {
    $obj = new SerializableObject();
    return array('a' => $obj, 'b' => $obj, 'c' => array('d' => $obj));
  }

}
var_dump(json_encode(new MultipleNonCircularReference()));

class SimpleRecursion implements JsonSerializable {

  public function jsonSerialize() {
    return array('foo' => $this);
  }

}

var_dump(json_encode(new SimpleRecursion()));
var_dump(json_last_error_msg());
var_dump(json_encode(new SimpleRecursion(), JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error_msg());

class MultilevelRecursion implements JsonSerializable {

  public function jsonSerialize() {
    return array(
      'Recursion' => array(
        'across' => array(
          'multiple' => array('levels' => $this)
        )
      )
    );
  }

}

var_dump(json_encode(new MultilevelRecursion(), JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error_msg());

class Circular implements JsonSerializable {

  public $d;

  public function jsonSerialize() {
    return $this->d;
  }

}

class Dependency implements JsonSerializable {

  public $c;

  public function jsonSerialize() {
    return $this->c;
  }

}

$c = new Circular();
$d = new Dependency();
$c->d = $d;
$d->c = $c;
var_dump(json_encode($c, JSON_PARTIAL_OUTPUT_ON_ERROR));
var_dump(json_last_error_msg());
