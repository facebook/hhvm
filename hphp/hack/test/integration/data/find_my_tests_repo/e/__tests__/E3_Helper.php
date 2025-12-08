<?hh

// Not a test file!

class E3_Helper {

  public static function helper1(): vec<int> {

    $values = vec[];
    foreach (E3_Sub::getValues() as $key => $name) {
      $values[] = $name;
    }
    return $values;

  }

}
