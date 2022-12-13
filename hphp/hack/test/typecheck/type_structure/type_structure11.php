<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface K { }

interface IEB {
  abstract const type TQ as IEQ;
}
interface IEQ {
  abstract const type TUQ as K;
}
class C<TSE as IEB, TQ> {
  public function test(classname<TSE> $cn):void where TQ = TSE::TQ {
    $x = type_structure($cn, 'TQ')['classname'] as nonnull;
//    $y = type_structure($x, 'TUQ')['classname'] as nonnull;
  }
}
