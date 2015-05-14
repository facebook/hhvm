<?hh //strict

interface I<+T> {
  public function get(): T;
}

class A {}
class B extends A {}

interface IAB extends I<A>, I<B> {
  public function get(): B;
}

interface IBA extends I<B>, I<A> {
  public function get(): B;
}

function takeA(I<A> $_): void {}
function takeB(I<B> $_): void {}

function test(IAB $ab, IBA $ba): void {
  takeA($ab);
  takeB($ab);
  takeA($ba);
  takeB($ba);
}
