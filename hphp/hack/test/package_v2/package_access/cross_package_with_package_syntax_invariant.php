//// pkg1.php
<?hh
function pkg1_call(): void {}

//// pkg4.php
<?hh
<<file: __PackageOverride('pkg4')>>
function pkg4_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
function test_err() : void {
    while (true) {
       invariant(package pkg1, "");
       pkg1_call(); // ok; pkg1 has been loaded
       pkg4_call(); // error; pkg4 has not yet been loaded

       invariant(package_exists("pkg1"), "");
       pkg4_call(); // error; pkg4 has not yet been loaded

       invariant(package pkg4, "");
       // both calls should be ok here
       pkg1_call();
       pkg4_call();

       break;
    }

    // error; access of pkg1 is outside the scope
    // of invariant(package pkg1, ...) statement
    pkg1_call();
 }

function test_ok() : void {
    invariant(package pkg1 && package pkg4, "");
    // both calls should be ok here
    pkg1_call();
    pkg4_call();
 }
