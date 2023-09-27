//// a.php
<?hh
enum E: int as int {
  A = 0;
  B = 1;
}
//// b.php
<?hh
enum F: int as int {
  C = 2;
}
//// c.php
<?hh
enum G: int as int {
  use F;
  D = 3;
}

//// h.php
<?hh
enum H: int as int {
  use G;
}

//// foo-f.php
<?hh
function foo_f(F $e): void {
  switch ($e) {
    case F::C:
      return;
  }
}

//// foo-g.php
<?hh
function foo_g(G $e): void {
  switch ($e) {
    case G::C:
    case G::D:
      return;
  }
}

//// foo-h.php
<?hh
function foo_h(H $e): void {
  switch ($e) {
    case H::C:
    case H::D:
      return;
  }
}

/////////////////

//// b.php
<?hh
enum F: int as int {
  use E;
  C = 2;
}
