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
  C = 2;
}
//// base-c.php
<?hh
enum G: int as int {
  use F;
  D = 3;
}

//// base-foo-f.php
<?hh
function foo_f(F $e): void {
  switch ($e) {
    case F::A:
    case F::B:
    case F::C:
      return;
  }
}
//// base-foo-g.php
<?hh
function foo_g(G $e): void {
  switch ($e) {
    case G::A:
    case G::B:
    case G::C:
    case G::D:
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
  C = 2;
}
//// changed-c.php
<?hh
enum G: int as int {
  use F;
  D = 3;
}

//// changed-foo-f.php
<?hh
function foo_f(F $e): void {
  switch ($e) {
    case F::A:
    case F::B:
    case F::C:
      return;
  }
}
//// changed-foo-g.php
<?hh
function foo_g(G $e): void {
  switch ($e) {
    case G::A:
    case G::B:
    case G::C:
    case G::D:
      return;
  }
}
