//// pkg7.php
<?hh
<<file: __PackageOverride('pkg7')>>
function pkg7_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (!(package pkg7)) {
    pkg7_call();   // error; pkg7 is not loaded
    return;
  } else {
    pkg7_call();  // ok
  }
  pkg7_call(); // ok, !(package pkg7) exited early
}
