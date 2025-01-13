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
      // neither is allowed because disjuction doesn't register package info
      pkg5_call();
      pkg4_call();
  }
}
