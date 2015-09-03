//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $var = 1;
    while (true) {
      echo "foo";
    }
    if (false) {
      echo "bar";
      return;
    } else {
      switch ($var) {
        case 1:
          echo "match this";
          return;
          break;
        case 2:
          echo "and this";
          return;
          break;
        case 3:
          echo "dont match this";
          break;
      }
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict


class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      switch ("__ANY") {
        case "__KSTAR":
        case "__ANY":
          "__KSTAR";
          return;
          break;
        case "__KSTAR":
      }
    }
    return;
  }
}
