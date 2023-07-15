<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

type AwaitablePredicate<T> = supportdyn<(function(T): Awaitable<~bool>)>;

<<__SupportDynamicType>>
function ho(IAsyncPredicate<C,int> $predicate): void { }

<<__SupportDynamicType>>
interface IAsyncPredicate<-T as supportdyn<mixed>, +Tc> { }

<<__SupportDynamicType>>
function asyncLambda<Tv as supportdyn<mixed>, Tc as supportdyn<mixed>>(
    AwaitablePredicate<Tv> $lambda,
  ): ~IAsyncPredicate<Tv,Tc> {
    throw new Exception("A");
  }

<<__SupportDynamicType>>
function expectString(string $_):void { }

<<__SupportDynamicType>>
function test(keyset<int> $_): void {
  ho(asyncLambda(async ($sig) ==> $sig->p));
  ho(asyncLambda(async ($s) ==> { expectString($s->foo()); return false; }));
  }


<<__SupportDynamicType>>
class C {
  public bool $p = false;
  public function foo():string { return "A"; }
}
