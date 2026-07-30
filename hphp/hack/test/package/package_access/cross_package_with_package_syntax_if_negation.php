//// pkg7/pkg7.php
<?hh
function pkg7_call(): void {}

//// pkg3/pkg3.php
<?hh
function test(): void {
  if (!(package pkg7)) {
    pkg7_call();   // error; pkg7 is not loaded
    return;
  } else {
    pkg7_call();  // ok
  }
  pkg7_call(); // ok, !(package pkg7) exited early
}
