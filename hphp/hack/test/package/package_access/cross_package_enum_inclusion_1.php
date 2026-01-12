//// foo.php
<?hh

// package pkg4 (disjoint from pkg1)
<<file: __PackageOverride('pkg4')>>

enum C : string {
  use D;
}

//// bar.php
<?hh
// package pkg1

enum D : string {}
