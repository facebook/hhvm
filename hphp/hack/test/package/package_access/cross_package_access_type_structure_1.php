//// a.php
<?hh
// package pkg

function test(): void {
  type_structure(B::class, 'T');
}

//// pkg2/b.php
<?hh
// package pkg2

class B {
  const type T = int;
}
