//// a.php
<?hh
// package pkg

function test(): void {
  type_structure(B1::class, "T");      // not error
}

//// pkg2/b.php
<?hh
// package pkg2

class B1 {
  const type T = string;
}
