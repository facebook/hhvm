<?hh

class Bar {
  public int $prop = 0;
}

function arbitrary_func(mixed $a, mixed $b): ?Bar {
  return null;
}

/* This is a test of recognizing the sigleton pattern for writing to global collections.
 * Specially, the static dictionary is accessed by functions "idx" or "C\contains_key".
 */
class Foo {
  private static dict<string, Bar> $static_dic = dict[];
  private static shape(
    'field1' => ?Bar,
    'field2' => ?Bar,
  ) $static_shape = shape(
    'field1' => null,
    'field2' => null,
  );

  public static function get_dict_value(string $key): Bar {
    $value = idx(self::$static_dic, $key);
    if ($value === null) {
      $value = new Bar();
      self::$static_dic[$key] = $value; // Caching
    }
    return $value;
  }

  public static function get_dict_value2(string $key): Bar {
    if (!HH\Lib\C\contains_key(self::$static_dic, $key)) {
      self::$static_dic[$key] = new Bar();  // Caching
    }
    return self::$static_dic[$key];
  }

  public static function get_shape_value(): Bar {
    $value = self::$static_shape["field1"];
    if ($value === null) {
      $value = new Bar();
      self::$static_shape["field1"] = $value; // Caching
    }
    return $value;
  }

  public static function get_dict_value_3(string $key): Bar {
    $value = arbitrary_func(self::$static_dic, $key);
    if ($value === null) {
      $value = new Bar();
      self::$static_dic[$key] = $value; // Not caching
    }
    return $value;
  }

  public static function get_dict_value_4(string $key): Bar {
    $exist_key = HH\Lib\C\contains_key(self::$static_dic, $key);

    if (!$exist_key) {
      self::$static_dic[$key] = new Bar();  // FN: caching not recognized
    }
    return self::$static_dic[$key];
  }
}
