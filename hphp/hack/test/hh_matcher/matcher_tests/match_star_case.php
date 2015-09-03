//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    switch (5) {
      case 0:
      case 1: echo "true";
      case 3:
        echo "something";
        break;
    }
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    switch (5) {
      case 0:
      case "__KSTAR":
    }
  }
}
