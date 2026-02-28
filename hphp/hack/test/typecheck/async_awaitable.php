<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function makeVec(): vec<D> { return vec[]; }
class D implements I { }

interface I { }

function makeAwaitable<T>(T $x): Awaitable<T> {
  throw new Exception("A");
}

function test():void {
  $f = async () ==> makeVec();
  // This works
  bar($f);
  // This also works
  bar<D>(async () ==> makeVec());
  // As does this
  bar(() ==> makeAwaitable(makeVec()));
  // This doesn't, when pessimised
  bar(async () ==> makeVec());
}

function bar<T as I>((function(): Awaitable<vec<T>>) $fn):void { }
