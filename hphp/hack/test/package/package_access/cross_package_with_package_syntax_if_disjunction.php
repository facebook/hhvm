//// pkg8/pkg8.php
<?hh
function pkg8_call(): void {}

//// pkg7/pkg7.php
<?hh
function pkg7_call(): void {}

//// pkg3/pkg3.php
<?hh
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
