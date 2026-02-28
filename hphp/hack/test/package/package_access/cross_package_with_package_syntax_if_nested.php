//// pkg1.php
<?hh
function pkg1_call(): void {}

//// pkg4.php
<?hh
<<file: __PackageOverride('pkg4')>>
function pkg4_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (package pkg1) {
    if (package pkg4) {
      // both pkg1 and pkg4 are accessible here
      pkg1_call();
      pkg4_call();
    }
    // only pkg1 is accessible here
    pkg1_call(); // ok
    pkg4_call(); // error
  }
}
