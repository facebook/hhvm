<?hh

type Id_of<+T> = int;

final class C1 {
  const type T = Id_of<this>;
}

class C2 {
  const type T = Id_of<this>;
}

function take_refied<reify T>(): void {}

function test(): void {
  // OK
  take_refied<C1::T>();
  // error
  take_refied<C2::T>();
}
