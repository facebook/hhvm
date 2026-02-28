<?hh

class Klass {
  public function foo(): void {
    $shadowed = true;
    $referenced = true; // should become a param to the extracted method
    /*range-start*/
    (() ==> {
      $shadowed = false || $referenced;
    });
    /*range-end*/
    $z = $shadowed;
  }
}
