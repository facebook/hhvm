<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
interface I<+T> { }

<<__SupportDynamicType>>
class Wrap<+T> implements I<T> {
  public function __construct(private T $item) { }
}

<<__SupportDynamicType>>
function getVecInt():~vec<int> { return vec[3]; }

<<__SupportDynamicType>>
function vectorAppend<Tv as supportdyn<mixed> >(Vector<Tv> $v, Tv $item):void { }

function vectorAppend_likes<Tv as supportdyn<mixed> >(~Vector<Tv> $v, ~Tv $item):void { }

function vectorAppend_likes_special(~Vector<~Wrap<int>> $v, ~Wrap<~int> $item):void { }

function vectorAppend_likes2<Tv as supportdyn<mixed> >(~Tv $item, ~Vector<Tv> $v):void { }

function test_transitive<T as I<int>>(~T $_):void { }

<<__SupportDynamicType>>
function makeVector<T as supportdyn<mixed>>():Vector<T> {
  return Vector<T>{};
}

function simple(~Vector<~Wrap<int>> $_):void { }


<<__SupportDynamicType>>
function test1():void {
  $x = makeVector<Wrap<int>>();
  $y = new Wrap(getVecInt()[0]);
  vectorAppend($x, $y); // vectorAppend<Wrap<int>> works
}

<<__SupportDynamicType>>
function test2():void {
  $x = makeVector<Wrap<int>>();
  $y = new Wrap(getVecInt()[0]);
  vectorAppend_likes($x, $y); // vectorAppend_likes<Wrap<int>> works
}

<<__SupportDynamicType>>
function test3():void {
  $x = makeVector<Wrap<int>>();
  $y = new Wrap(getVecInt()[0]);
  vectorAppend_likes2($y, $x); // vectorAppend_likes2<Wrap<int>> works
}

function test4():void {
  $x = new Wrap(getVecInt()[0]);
  test_transitive($x);
}
