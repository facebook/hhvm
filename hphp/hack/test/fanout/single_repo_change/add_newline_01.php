//// base-a.php
<?hh

class C {
  public function foo(): void {}
}

function zot(): void {}

//// base-u.php
<?hh

function bar(C $c): void {
  $c->foo();
  zot();
}

//// changed-a.php
<?hh


class C {
  public function foo(): void {}
}

function zot(): void {}

//// changed-u.php
<?hh

function bar(C $c): void {
  $c->foo();
  zot();
}
