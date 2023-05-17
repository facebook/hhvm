<?hh

class Klass {
  public function foo(): void {
    $z = nonexistent();
    /*range-start*/
    $x = $z->meth($z);/*range-end*/
    $x + 2;
  }
}
