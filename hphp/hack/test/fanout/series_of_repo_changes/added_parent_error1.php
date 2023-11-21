//// i.php
<?hh

interface I {}

//// a.php
<?hh

class A {}

//// b.php
<?hh

class B extends A {}

//// f.php

function f(B $x): I {
  return $x;
}

//////////////////////

//// a.php
<?hh

class A implements I {}
