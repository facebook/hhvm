//// pkg4/foo.php
<?hh

// package pkg4 (disjoint from pkg1) by path

enum C : string {
  use D;
}

//// bar.php
<?hh
// package pkg1

enum D : string {}
