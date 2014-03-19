//// FileA.php
<?hh

class Foo {
  public static function bar(): array {
    return array();
  }

  public static function duck() {
  }
}

//// FileB.php
<?hh // strict

class Bar {
  public function foo1(): int {
    // Allow the Tany from an untemplated array
    $arr = Foo::bar();
    return $arr[0];
  }

  public function foo2(): int {
    // Allow the Tany from a missing return type
    $arr = Foo::duck();
    return $arr[0];
  }

  public function foo3(): void {
    // Allow the Tany from an untemplated array
    $arr = Foo::bar();
    $arr[] = 123;
  }

  public function foo4(): void {
    // Allow the Tany from a missing return type
    $arr = Foo::duck();
    $arr[] = 123;
  }
}
