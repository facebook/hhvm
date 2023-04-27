//// base-a.php
<?hh
enum E: int as int {
  A = 0;
  B = 1;
}
//// base-b.php
<?hh
enum F: int as int {
  use E;
}
//// base-foo.php
<?hh
function foo(F $e): void {
  switch ($e) {
    case F::A:
    case F::B:
      return;
  }

}

//// changed-a.php
<?hh
enum E: int as int {
  A = 0;
  B = 1;
  C = 2;
}
//// changed-b.php
<?hh
enum F: int as int {
  use E;
}
//// changed-foo.php
<?hh
function foo(F $e): void {
  switch ($e) {
    case F::A:
    case F::B:
      return;
  }
}
