//// Infer.php
<?hh // strict

class Infer {
  public function sample(int $k): int {
    return 0;
  }
}

//// PPL.php
<?hh // strict

function sample(string $k): void {}

class MyClass {
  public function test(): void {
    sample("hi");
  }
}
