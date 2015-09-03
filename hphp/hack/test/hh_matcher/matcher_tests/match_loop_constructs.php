//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    while (true) {
      if (true) {
        break;
      } else {
        continue;
      }
    }
  }
}

//// matcherpattern.php
<?hh //strict
class Foo {
  public function bar(): void {
    while (true) {
      if (true) {
        break;
      } else {
        continue;
      }
    }
  }
}
