//// foo.php
<?hh

// package pkg1
class C2 {
  public function foo(): void {}
}

class N1 {} // this extra class silences the filename linter

//// bar.php
<?hh

// package pkg1
class D2 {
  public function bar(): void {
    C2::class; // should NOT raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
