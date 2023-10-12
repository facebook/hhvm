<?hh // strict

class C {
  const type T = int;
}

function test(): void {
  type_structure('C', 'T');
}
