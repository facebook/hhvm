//// FileA.php
<?hh

class C {
  <<__DynamicallyCallable>>
  public function foo(): void {}
}

//// FileB.php
<?hh

class D extends C {
  <<__Override>>
  public function foo(): void {}
}
