//// a.php
<?hh
// package pkg

function test(): void {
  type_structure(B::class, 'T');
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

class B {
  const type T = int;
}
