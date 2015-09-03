//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    array("array one", "more elements");
    array("map array" => "mapped_elem", "elem 2" => "ele2");
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    array("array one", "more elements");
    array("map array" => "mapped_elem", "elem 2" => "ele2");
  }
}
