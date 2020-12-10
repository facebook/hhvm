<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface IPure {
  <<__Pure>>
  public function pure(): void;
}
class IPureChild implements IPure {
  public function pure(): void {}
}
