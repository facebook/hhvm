<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  public static function foo():void { }
}

<<__SupportDynamicType>>
interface Get {
  public function get():string;
}

<<__SupportDynamicType>>
class D implements Get {
  public function get():(~classname<C> & string) {
    return C::class;
  }
}

<<__SupportDynamicType>>
function test(vec<int> $_):void {
  $d = new D();
  $c = $d->get();
  $c::foo();
}
