//// tosearch.php
<?hh //strict

class Bar {
  public function baz(): void {
    echo "true";
  }
}

//// matcherpattern.php
<?hh //strict

class Bar {
  public function __ANY(): void {}
}
