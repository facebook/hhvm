//// tosearch.php
<?hh //strict

class Bar {
  public function foo(): void {
    echo "true";
  }
}

//// matcherpattern.php
<?hh //strict

class Bar {
  public function __SOMENAME(): void {
    echo "true";
  }
}
