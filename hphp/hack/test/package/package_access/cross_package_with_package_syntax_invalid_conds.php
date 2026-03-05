//// pkg8.php
<?hh
<<file: __PackageOverride('pkg8')>>
function pkg8_call(): void {}

//// pkg7.php
<?hh
<<file: __PackageOverride('pkg7')>>
function pkg7_call(): void {}

//// pkg3.php
<?hh
<<file: __PackageOverride('pkg3')>>
class C {
    public function test() : void {
      if (package pkg8 == package pkg7) {
        pkg8_call(); // error; comparison binops don't register package info
      }
      if (package pkg8 is bool) {
        pkg8_call(); // error; type ops don't register package info
      }
      if ($this->expect(package pkg8)) {
        pkg8_call(); // error; function calls don't register package info
      }
      if ($loaded = package pkg8) {
        pkg8_call(); // error; assignments aren't allowed in conditionals

        if ($loaded) {
          pkg8_call(); // error; cannot infer $loaded to be a package expression
        }
      }
    }

    private function expect(bool $x) : bool {
      return $x;
    }
  }
