<?hh

class C {
  const type T = int;
}

function test(): void {
  $x = new C();
  type_structure($x, 'T');
}
