//// file1.php
<?hh
trait MyTrait {
  use MyOtherTrait;
  public function foo(): int {
    return 1;
  }
  // should be part of the fanout because this has an error now.
}

//// othertrait.php
<?hh

trait MyOtherTrait {
  require implements MyInterface;
}

//// file2.php
<?hh

interface MyInterface {}

///////////////////////////////

//// file2.php
<?hh

interface MyInterface {
  public function foo(): float;
}
