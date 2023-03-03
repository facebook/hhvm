<?hh

class NadMe1 {}
class NadMe2 extends NadMe1 {}
class NadMe3 extends NadMe2 {}

class SdMe1 {
  public static function foo(vec<int> $_): void {}
}
class SdMe2 extends SdMe1 {}
final class SdMe3 extends SdMe2 {}

function main(dynamic $d): void {
  SdMe1::foo($d);
}
