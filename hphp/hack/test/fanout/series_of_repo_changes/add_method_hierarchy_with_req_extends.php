//// a.php
<?hh

class A {}

//// b.php
<?hh

trait B {
  require extends A;
}

//// c.php
<?hh

trait C {
  use B;
}

//// d.php
<?hh

class D {
  use C;
}

//// f.php
<?hh

class F extends A {
  use C;
}

//// e.php
<?hh

trait E {
  use B;
  public function foo(): void {}
}

//// fa.php
<?hh

function fa(A $x): void {
  $x->foo();
}

//// fb.php
<?hh

trait B2 {
  require extends A;

  public function f(): void {
    $this->foo();
  }
}

//// fc.php
<?hh

trait C2 {
  use B;

  public function f(): void {
    $this->foo();
  }
}

//// fd.php
<?hh

function fd(D $x): void {
  $x->foo();
}

//// ff.php
<?hh

function ff(F $x): void {
  $x->foo();
}

//// fe.php
<?hh

trait E2 {
  use B;
  public function foo(): void {}

  public function f(): void {
    $this->foo();
  }
}

/////////////////////////////

//// a.php
<?hh

class A {
  public function foo(): void {}
}
