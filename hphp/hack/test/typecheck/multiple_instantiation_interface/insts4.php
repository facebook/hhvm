<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface IEJTO { }
interface IEJTI { }
interface IVC extends IVCB { }
interface IVCB { }
// Permitted because IESVMB<IVC,ERE> <: IESBMV<IVC,IEJTI>
// *and* IESVMB<IVC,ERE> <: IESVMB<IVC<IEJTO>
// which are implemented by IEJTIMV and IEJTOMV
// take away the first decl, and this is illegal
// But also: move the first decl, and it's illegal
interface IEREMV extends IESBMV<IVC,ERE>, IEJTIMV, IEJTOMV { }
interface IESBMV<TVC as IVCB,+TE> extends IEMVB<TVC> { }
interface IEMVB<TVC as IVCB> {
  public function getVC(): TVC;
}
interface IEJTIMV extends IESBMV<IVC,IEJTI> {
  public function foo1():void;
}
interface IEJTOMV extends IESBMV<IVC,IEJTO> {
  public function foo2():void;
}
final class ERE {
  use EREAT;
}

trait EREAT implements IEJTO, IEJTI { }

function expect1(IESBMV<IVC,ERE> $x):void { expect2($x); expect3($x); }
function expect2(IESBMV<IVC,IEJTO> $x):void { }
function expect3(IESBMV<IVC,IEJTI> $x):void { }
function pass1(IEREMV $x):void {
  expect1($x);
  expect2($x);
  expect3($x);
}
