<?hh

class SerializableObject implements JsonSerializable {

  public function jsonSerialize() :mixed{
    return dict['foo' => 'bar'];
  }

}

class MultipleNonCircularReference implements JsonSerializable {

  public function jsonSerialize() :mixed{
    $obj = new SerializableObject();
    return dict['a' => $obj, 'b' => $obj, 'c' => dict['d' => $obj]];
  }

}

class SimpleRecursion implements JsonSerializable {

  public function jsonSerialize() :mixed{
    return dict['foo' => $this];
  }

}

class MultilevelRecursion implements JsonSerializable {

  public function jsonSerialize() :mixed{
    return dict[
      'Recursion' => dict[
        'across' => dict[
          'multiple' => dict['levels' => $this]
        ]
      ]
    ];
  }

}

class Circular implements JsonSerializable {

  public $d;

  public function jsonSerialize() :mixed{
    return $this->d;
  }

}

class Dependency implements JsonSerializable {

  public $c;

  public function jsonSerialize() :mixed{
    return $this->c;
  }

}


<<__EntryPoint>>
function main_json_encode() :mixed{
  var_dump(json_encode(dict["a" => 1, "b" => 2.3, 3 => "test"]));
  var_dump(json_encode(vec["a", 1, true, false, null]));

  var_dump(json_encode("a\xE0"));
  var_dump(json_encode("a\xE0", JSON_FB_LOOSE));

  var_dump(json_encode(dict["0" => "apple", "1" => "banana"]));

  var_dump(json_encode(vec[dict["a" => "apple"]]));

  var_dump(json_encode(vec[dict["a" => "apple"]], JSON_PRETTY_PRINT));

  var_dump(json_encode(vec[1, 2, 3, vec[1, 2, 3]], JSON_PRETTY_PRINT));

  $arr = dict[
    "a" => 1,
    "b" => vec[1, 2],
    "c" => dict["d" => 42]
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
