<?hh

class C<<<__Explicit>> T> extends Exception{}

function test() : void {
  try {
    throw new C<int>();
  } catch (C $c) {
    throw $c;
  }
}
