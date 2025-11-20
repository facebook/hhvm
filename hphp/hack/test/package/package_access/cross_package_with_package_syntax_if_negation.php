//// pkg4.php
<?hh
<<file: __PackageOverride('pkg4')>>
function pkg4_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (!(package pkg4)) {
    pkg4_call();   // error; pkg4 is not loaded
    return;
  } else {
    pkg4_call();  // ok
  }
  pkg4_call(); // error; pakcage info unknown outside if/else branches
}
