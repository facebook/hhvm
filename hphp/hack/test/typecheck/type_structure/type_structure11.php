<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
<<file: __EnableUnstableFeatures('class_type')>>
interface K {}

interface IEB {
  abstract const type TQ as IEQ;
}
interface IEQ {
  abstract const type TUQ as K;
}
class C<TSE as IEB with { type TQ = TQ }, TQ as IEQ> {
  public function test(class<TSE> $cn): void {
    $x = type_structure($cn, 'TQ')['classname'] as nonnull;
    //    $y = type_structure($x, 'TUQ')['classname'] as nonnull;
  }
}
