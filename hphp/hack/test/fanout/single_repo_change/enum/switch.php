//// base-a.php
<?hh
enum E: int as int {
  A = 0;
  B = 1;
}
//// base-b.php
<?hh
function foo(E $e): void {
  switch ($e) {
    case E::A:
    case E::B:
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
function foo(E $e): void {
  switch ($e) {
    case E::A:
    case E::B:
      return;
  }
}
