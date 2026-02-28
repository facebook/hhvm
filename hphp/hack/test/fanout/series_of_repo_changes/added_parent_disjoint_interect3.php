//// a.php
<?hh

class A {}

//// b.php
<?hh

class B {}

//// c.php
<?hh

class C extends A {}

//// f.php
<?hh

function f(C $x): void {
  if ($x is B) {
    expect_string($x);
  }
}

//// expect_string.php
<?hh

function expect_string(string $_): void {}

////////////////////////

//// a.php
<?hh

class A extends B {}
