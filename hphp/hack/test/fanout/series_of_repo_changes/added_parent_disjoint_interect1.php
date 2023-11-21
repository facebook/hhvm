//// a.php
<?hh

class A {}

//// b.php
<?hh

class B {}

//// f.php
<?hh

function f(A $a): void {
  if ($a is B) {
    expect_string($a);
  }
}

//// expect_string.php
<?hh

function expect_string(string $_): void {}

////////////////////////

//// a.php
<?hh

class A extends B {}
