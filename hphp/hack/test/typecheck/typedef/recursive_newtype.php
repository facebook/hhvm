//// f1.php
<?hh

type T1 = ?T2;

newtype N1 = ?N2;
//// f2.php
<?hh

type T2 = T1;

newtype N2 = N1;
//// f3.php
<?hh

function f(T1 $t): mixed {
  return $t;
}
