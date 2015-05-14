<?hh //strict

interface I<+T> {
  public function get(): T;
}

interface A {}
interface B {}

interface ISum extends A, B {}

interface I1 extends I<A>, I<B>, I<ISum> {
  public function get(): ISum;
}

interface I2 extends I<A>, I<ISum>, I<B> {
  public function get(): ISum;
}

interface I3 extends I<ISum>, I<A>, I<B> {
  public function get(): ISum;
}

function takeSum(I<ISum> $_): void {}

function test(I1 $i1, I2 $i2, I3 $i3): void {
  takeSum($i1);
  takeSum($i2);
  takeSum($i3);
}
