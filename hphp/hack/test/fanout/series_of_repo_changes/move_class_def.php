//// file1.php
<?hh

class A {}

//// file2.php
<?hh

//// b.php
<?hh

class B extends A {}

//// f.php
<?hh

function f(B $x): B {
  return $x;
}

//// g.php
<?hh

function g(A $a): A {
  return $a;
}

///////////////

//// file1.php
<?hh

//// file2.php
<?hh

class A {}
