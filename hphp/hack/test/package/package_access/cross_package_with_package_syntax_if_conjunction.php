//// pkg7.php
<?hh
<<file: __PackageOverride('pkg7')>>
function pkg7_call(): void {}

//// pkg8.php
<?hh
<<file: __PackageOverride('pkg8')>>
function pkg8_call(): void {}

//// pkg3.php  (all packages are disjoint)
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (package pkg7 && package pkg8) {
    // both are allowed
    pkg7_call();
    pkg8_call();
  } else {
    // both are rejected
    pkg7_call();
    pkg8_call();
  }
  // both are rejected too
  pkg7_call();
  pkg8_call();
}
