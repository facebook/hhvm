//// a.php
<?hh
// package pkg

function test(): void {
  B1::FOO;      // error
}

//// pkg2/b.php
<?hh
// package pkg2

class B1 {
  const string FOO = 'foo';
}
