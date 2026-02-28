<?hh

class Klass {
  public static function foo(): void {
    // The extracted function is static iff the function we are extracting from is static.
    /*range-start*/
    $return = 100;
    /*range-end*/
    var_dump($return);
  }
}
