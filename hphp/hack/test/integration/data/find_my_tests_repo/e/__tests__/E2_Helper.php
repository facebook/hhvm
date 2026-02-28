<?hh

// Not a test file!

class E2_Helper {

  public static function helper1(): vec<int> {

    $values = vec[];
    foreach (E2::getValues() as $key => $name) {
      $values[] = $name;
    }
    return $values;

  }

}
