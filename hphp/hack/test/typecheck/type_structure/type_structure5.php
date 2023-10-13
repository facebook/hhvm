<?hh

class C {}

function test(): void {
  type_structure(C::class, 123);
}
