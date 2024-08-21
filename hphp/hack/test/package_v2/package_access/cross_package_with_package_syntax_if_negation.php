//// pkg1.php
<?hh
function pkg1_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (!(package pkg1)) {
    pkg1_call();   // error; pkg1 is not loaded
    return;
  } else {
    pkg1_call();  // ok
  }
  pkg1_call(); // error; pakcage info unknown outside if/else branches
}
