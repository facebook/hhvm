<?hh

class Klass {
  public function one(): int {
    return 1;
  }
  public function foo(): void {
    /*range-start*/
    // because `$this` is used, we generate a nonstatic method
    $return = $this->one();
    /*range-end*/
    var_dump($return);
  }
}
