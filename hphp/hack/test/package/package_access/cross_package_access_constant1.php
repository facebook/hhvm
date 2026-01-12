//// a.php
<?hh
// package pkg

function test(): void {
  B1::FOO;      // error
}

//// b.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>

class B1 {
  const string FOO = 'foo';
}
