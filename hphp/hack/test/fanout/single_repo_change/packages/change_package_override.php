//// base-a.php
<?hh

<<file: __PackageOverride('intern')>>

class C {
  public function foo(): void {}
}

//// base-u.php
<?hh

function bar(C $c): void {
  $c->foo();
}

//// changed-a.php
<?hh

class C {
  public function foo(): void {}
}

//// changed-u.php
<?hh

function bar(C $c): void {
  $c->foo();
}
