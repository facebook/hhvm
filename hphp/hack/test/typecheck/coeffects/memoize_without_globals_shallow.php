<?hh
class Foo implements IMemoizeParam {

  public function getInstanceKey()[]: string {
    return "";
  }
}
abstract class Bar implements IMemoizeParam  {
  abstract const ctx CFoo as [zoned_shallow];
  public function getInstanceKey()[]: string {
    return "";
  }

}

<<__Memoize>>
  function foo(Foo $x)[zoned_shallow]: void {} // ok

<<__Memoize>>
function bar(Foo $x)[zoned_local]: void {} // ok

<<__Memoize>>
function baz(Foo $z)[zoned]: void {} // still bad

<<__Memoize>>
function bat(Bar $a)[$a::CFoo] : void {}
