<?hh

abstract class C {

  public static function f1(): SomeMap {
    foreach($type_map as $key => ) { // missing '$value'
      $type_string_map[(string)$key] =
        Str\lowercase((string)str_replace('_', ' ', (string)$value));
    }
    return new SomeMap(
      new Map($type_string_map)
    );
  }

  public static function f2():SomeMap {
    $other_type_map = Numbers::getValues();
    $other_type_string_map = Map {};
    foreach($other_type_map as $key => $value) {
      $other_type_string_map[(string)$key] =
        Str\lowercase((string)str_replace('_', ' ', (string)$value));
    }
    return new SomeMap(
      new Map($other_type_string_map)
    );
  }
}
