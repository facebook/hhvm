//// pkg7/pkg7.php
<?hh
function pkg7_call(): void {}

//// pkg2/pkg2.php
<?hh
function pkg2_call(): void {}

//// pkg3/pkg3.php
<?hh
function pkg3_call(): void {}

function test_do_while() : void {
  do {
    if (package pkg7) {
      pkg7_call();
    } else {
      pkg3_call();
    }
  } while (package pkg7);
}

function test_while() : void {
    while (!(package pkg7)) {
    pkg2_call(); // ok; pkg3 includes pkg2
  };

  pkg7_call(); // error; package info doesn't transfer after while statement
}
