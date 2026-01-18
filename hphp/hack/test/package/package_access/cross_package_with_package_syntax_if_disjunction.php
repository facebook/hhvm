//// pkg5.php
<?hh
<<file: __PackageOverride('pkg5')>>
function pkg5_call(): void {}

//// pkg4.php
<?hh
<<file: __PackageOverride('pkg4')>>
function pkg4_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test(): void {
  if (package pkg4 || package pkg5) {
    // neither is allowed as we cannot be sure
    pkg5_call();
    pkg4_call();
  } else {
    // similarly here, both are rejected
    pkg5_call();
    pkg4_call();
  }
  // and here, both are rejected
  pkg5_call();
  pkg4_call();
}
