<?hh

namespace AnnoyingNS;

<<\NoArgAttr>>
final class TerribleClass {

  <<__Memoize, \TwoArgMethodAttr(\AppleThenBanana::class, 42)>>
  public function terribleMethod(): void {}

}
