//// pkg5.php
<?hh
<<file: __PackageOverride('pkg5')>>
function pkg5_call(): void {}

//// pkg4.php
<?hh
<<file: __PackageOverride('pkg4')>>
function pkg4_call(): void {}

//// pkg3.php  (all packages are disjoint)
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (package pkg4 && package pkg5) {
    // both are allowed
    pkg4_call();
    pkg5_call();
  } else {
    // both are rejected
    pkg4_call();
    pkg5_call();
  }
  // both are rejected too
  pkg4_call();
  pkg5_call();
}
