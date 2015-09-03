//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    if (true) {}
    while (true) {
      echo "things";
    }
    throw new Exception ("Anexception");
    "more things";
    $some_assignment = 5;
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__KSTAR";
    throw new Exception ("Anexception");
    "__KSTAR";
    return;
  }
}
