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
class C {
    public function test() : void {
      if (package pkg1 == package pkg4) {
        pkg1_call(); // error; comparison binops don't register package info
      }
      if (package pkg1 is bool) {
        pkg1_call(); // error; type ops don't register package info
      }
      if ($this->expect(package pkg1)) {
        pkg1_call(); // error; function calls don't register package info
      }
      if ($loaded = package pkg1) {
        pkg1_call(); // error; assignments aren't allowed in conditionals

        if ($loaded) {
          pkg1_call(); // error; cannot infer $loaded to be a package expression
        }
      }
    }

    private function expect(bool $x) : bool {
      return $x;
    }
  }
