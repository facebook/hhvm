//// b.php
<?hh
<<file: __PackageOverride('pkg2')>>
function pkg2_call(): void {}

//// c.php
<?hh
<<file: __PackageOverride('pkg3')>>
function pkg3_call(): void {}

//// a.php
<?hh
// package pkg1

class A {
  <<__RequirePackage("pkg3")>>
  public function mfoo(): void {
    pkg3_call();
    pkg2_call(); // ok, because pkg3 includes pkg2
    pkg1_call();
  }

  public function mzot(): void {
    if (package pkg3) {
      pkg3_call();
      pkg2_call(); // ok, because pkg3 includes pkg2
      if (package pkg2) {
        pkg2_call(); // ok as above, the if package is redundant
      }
      pkg1_call();
    }
  }
}

<<__RequirePackage("pkg3")>>
function foo(): void {
  pkg3_call();
  pkg2_call(); // ok, because pkg3 includes pkg2
  if (package pkg2) {
    pkg2_call(); // ok as above, the if package is redundant
  }
  pkg1_call();
}


function zot(): void {
  if (package pkg3) {
    pkg3_call();
    pkg2_call(); // ok, because pkg3 includes pkg2
    if (package pkg2) {
      pkg2_call(); // ok as above, the if package is redundant
    }
    pkg1_call();
  }
}

function pkg1_call(): void {}
