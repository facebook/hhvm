<?hh

<<__SupportDynamicType>>
interface I { }

<<__SupportDynamicType>>
class C {
  public function __construct(
    private ~supportdyn<(function(
      I...
    ): ~Awaitable<void>)> $fn):void {}
  }

<<__SupportDynamicType>>
function foo(vec<int> $_):void {
  $f = async (I... $a) ==> { };
  // This works
  new C($f);
  // This doesn't
  new C(async (I... $a) ==> { }) ;
}
