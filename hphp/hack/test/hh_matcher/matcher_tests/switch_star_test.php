//// tosearch.php
<?hh //strict

class Foo {
  public function foo() : void {
    $anint = 5;
    switch ($anint) {
      case 1:
        echo "1";
        break;
      case 2:
        echo "1";
        return;
        break;
      case 4:
        echo " () (";
        // FALLTHROUGH
      default:
        echo "hi";
        break;
    }
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function foo() : void {
    $anint = 5;
    switch ($anint) {
      case "__KSTAR":
      case "__ANY":
        "__KSTAR";
        return;
        break;
      case "__KSTAR":
    }
  }
}
