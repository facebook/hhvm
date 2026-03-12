//// ptr5a.php
<?hh

// package pkg2 but override pkg1
<<file: __PackageOverride('pkg1')>>

class C2 {
  const int FOO = 3;
}

class N1 {} // this extra class silences the filename linter

//// ptr5b.php
<?hh

// package pkg2 but override pkg1
<<file: __PackageOverride('pkg1')>>

class D2 {
  public function bar(): void {
    C2::class; // should NOT raise a lint error
  }
}

class N2 {} // this extra class silences the filename linter
