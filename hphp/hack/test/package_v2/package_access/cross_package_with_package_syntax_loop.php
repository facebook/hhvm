//// pkg1.php
<?hh
function pkg1_call(): void {}

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
    if (package pkg1) {
      pkg1_call();
    } else {
      pkg3_call();
    }
  } while (package pkg1);
}

function test_while() : void {
    while (!(package pkg1)) {
    pkg2_call(); // ok; pkg3 includes pkg2
  };

  pkg1_call(); // error; package info doesn't transfer after while statement
}
