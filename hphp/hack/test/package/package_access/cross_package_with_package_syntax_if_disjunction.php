//// pkg8.php
<?hh
<<file: __PackageOverride('pkg8')>>
function pkg8_call(): void {}

//// pkg7.php
<?hh
<<file: __PackageOverride('pkg7')>>
function pkg7_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (package pkg7 || package pkg8) {
    // neither is allowed as we cannot be sure
    pkg8_call();
    pkg7_call();
  } else {
    // similarly here, both are rejected
    pkg8_call();
    pkg7_call();
  }
  // and here, both are rejected
  pkg8_call();
  pkg7_call();
}
