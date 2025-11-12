//// base-a.php
<?hh

class C {
  <<__Sealed("A::class")>>
  public function foo(): void {}
}

//// changed-a.php
<?hh

class C {
  <<__Sealed("B::class")>>
  public function foo(): void {}
}
