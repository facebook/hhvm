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
class C {
    public function test() : void {
      if (package pkg5 == package pkg4) {
        pkg5_call(); // error; comparison binops don't register package info
      }
      if (package pkg5 is bool) {
        pkg5_call(); // error; type ops don't register package info
      }
      if ($this->expect(package pkg5)) {
        pkg5_call(); // error; function calls don't register package info
      }
      if ($loaded = package pkg5) {
        pkg5_call(); // error; assignments aren't allowed in conditionals

        if ($loaded) {
          pkg5_call(); // error; cannot infer $loaded to be a package expression
        }
      }
    }

    private function expect(bool $x) : bool {
      return $x;
    }
  }
