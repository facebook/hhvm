//// base-e.php
<?hh
enum DcE: int as int {
  A = 0;
  B = 1;
}
//// base-f.php
<?hh
enum DcF: int as int {
  use DcE;
  C = 2;
}

//// changed-e.php
<?hh
<<__AllowUncheckedEnumValues>>
enum DcE: int as int {
  A = 0;
  B = 1;
}
//// changed-f.php
<?hh
enum DcF: int as int {
  use DcE;
  C = 2;
}
