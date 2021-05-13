<?hh

class SerializableObject implements JsonSerializable {

  public function jsonSerialize() {
    return darray['foo' => 'bar'];
  }

}

class MultipleNonCircularReference implements JsonSerializable {

  public function jsonSerialize() {
    $obj = new SerializableObject();
    return darray['a' => $obj, 'b' => $obj, 'c' => darray['d' => $obj]];
  }

}

class SimpleRecursion implements JsonSerializable {

  public function jsonSerialize() {
    return darray['foo' => $this];
  }

}

class MultilevelRecursion implements JsonSerializable {

  public function jsonSerialize() {
    return darray[
      'Recursion' => darray[
        'across' => darray[
          'multiple' => darray['levels' => $this]
        ]
      ]
    ];
  }

}

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


<<__EntryPoint>>
function main_json_encode() {
  var_dump(json_encode(darray["a" => 1, "b" => 2.3, 3 => "test"]));
  var_dump(json_encode(varray["a", 1, true, false, null]));

  var_dump(json_encode("a\xE0"));
  var_dump(json_encode("a\xE0", JSON_FB_LOOSE));

  var_dump(json_encode(darray["0" => "apple", "1" => "banana"]));

  var_dump(json_encode(varray[darray["a" => "apple"]]));

  var_dump(json_encode(varray[darray["a" => "apple"]], JSON_PRETTY_PRINT));

  var_dump(json_encode(varray[1, 2, 3, varray[1, 2, 3]], JSON_PRETTY_PRINT));

  $arr = darray[
    "a" => 1,
    "b" => varray[1, 2],
    "c" => darray["d" => 42]
  ];
  var_dump(json_encode($arr, JSON_PRETTY_PRINT));
  var_dump(json_encode(new SerializableObject()));
  var_dump(json_encode(new MultipleNonCircularReference()));

  $error = null;
  $result = json_encode_with_error(new SimpleRecursion(), inout $error);
  var_dump($result);
  var_dump($error[1] ?? 'No error');
  $error = null;
  $result = json_encode_with_error(
    new SimpleRecursion(),
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    new MultilevelRecursion(),
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[1] ?? 'No error');

  $c = new Circular();
  $d = new Dependency();
  $c->d = $d;
  $d->c = $c;
  $error = null;
  $result = json_encode_with_error($c, inout $error, JSON_PARTIAL_OUTPUT_ON_ERROR);
  var_dump($result);
  var_dump($error[1] ?? 'No error');
}
