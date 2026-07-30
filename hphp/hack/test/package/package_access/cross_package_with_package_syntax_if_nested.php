//// pkg1.php
<?hh
function pkg1_call(): void {}

//// pkg7/pkg7.php
<?hh
function pkg7_call(): void {}

//// pkg3/pkg3.php
<?hh
function test(): void {
  if (package pkg1) {
    if (package pkg7) {
      // both pkg1 and pkg7 are accessible here
      pkg1_call();
      pkg7_call();
    }
    // only pkg1 is accessible here
    pkg1_call(); // ok
    pkg7_call(); // error
  }
}
