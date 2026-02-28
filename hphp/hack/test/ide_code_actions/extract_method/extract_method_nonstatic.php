<?hh

class Klass {
  public function one(): int {
    return 1;
  }
  public function foo(): void {
    // The extracted function is static iff the function we are extracting from is static.
    /*range-start*/
    $return = $this->one();
    /*range-end*/
    var_dump($return);
  }
}
