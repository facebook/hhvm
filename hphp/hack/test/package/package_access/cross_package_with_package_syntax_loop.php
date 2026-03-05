//// pkg7.php
<?hh
<<file: __PackageOverride('pkg7')>>
function pkg7_call(): void {}

//// pkg2.php
<?hh
<<file: __PackageOverride('pkg2')>>
function pkg2_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
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
