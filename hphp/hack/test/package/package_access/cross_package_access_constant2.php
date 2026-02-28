//// a.php
<?hh
// package pkg

function test(): void {
  type_structure(B1::class, "T");      // not error
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

class B1 {
  const type T = string;
}
