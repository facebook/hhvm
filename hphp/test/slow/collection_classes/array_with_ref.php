<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// Make sure array to container fast-paths properly account for refs in the
// arrays.

class Foo {
  public function __construct($name) {
    $this->name = $name;
  }
  public function get() { return $this->name; }
}
function test_map(&$ref, $array, $key) {
  $map = new Map($array);
  echo "  map: ";
  var_dump($map[$key]->get());
}

function test_vector(&$ref, $array, $key) {
  $vector = new Vector($array);
  echo "  vector: ";
  var_dump($vector[$key]->get());
}

function test_set(&$ref, $array, $key) {
  $set = new Set($array);
  echo "  set: ";
  var_dump(isset($set[$key]));
}

function mixed() {
  echo "mixed:\n";
  $key1 =  new Foo("value1");
  $array = ["key0" => new Foo("value0"), "key1" => &$key1,
            "key2" => new Foo("value2"), "key3" => new Foo("value3")];
  test_map(&$key1, $array, "key1");
  test_vector(&$key1, $array, 1);

  $value1 = "value1";
  $set_array = ["key0" => "value0", "key1" => &$value1,
                "key2" => "value2", "key3" => "value3"];
  test_set(&$value1, $set_array, "value1");
}

function packed() {
  echo "packed:\n";
  $value1 = new Foo("value1");
  $array = [new Foo("value0"), &$value1,
            new Foo("value2"), new Foo("value3")];
  test_map(&$value1, $array, 1);
  test_vector(&$value1, $array, 1);

  $value1 = "value1";
  $set_array = ["value0", &$value1, "value2", "value3"];
  test_set(&$value1, $set_array, "value1");
}

<<__EntryPoint>>
function main_array_with_ref() {
mixed();
packed();
}
