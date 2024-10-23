//// base-file1.php
<?hh

class A {}

//// base-file2.php
<?hh

//// base-b.php
<?hh

class B extends A {}

//// base-f.php
<?hh

function f(B $x): B {
  return $x;
}

//// base-g.php
<?hh

function g(A $a): A {
  return $a;
}

//// changed-file1.php
<?hh

//// changed-file2.php
<?hh

class A {}

//// changed-b.php
<?hh

class B extends A {}

//// changed-f.php
<?hh

function f(B $x): B {
  return $x;
}

//// changed-g.php
<?hh

function g(A $a): A {
  return $a;
}
