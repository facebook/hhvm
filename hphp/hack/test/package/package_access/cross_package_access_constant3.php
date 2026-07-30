//// a.php
<?hh
// package pkg

function test(): void {
  $x = B1::class;
  $x::FOO;
}

//// pkg2/b.php
<?hh
// package pkg2

class B1 {
  const string FOO = 'foo';
}
