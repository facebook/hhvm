<?hh

type MyTypename = int;

function test(): void {
  type_structure(MyTypename::class, 'Nonsense');
}
