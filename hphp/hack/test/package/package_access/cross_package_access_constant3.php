//// a.php
<?hh
// package pkg

function test(): void {
  $x = B1::class;
  $x::FOO;
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

class B1 {
  const string FOO = 'foo';
}
