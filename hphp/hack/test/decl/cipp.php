<?hh // strict

class C {
  <<__Cipp(C::class)>>
  public function cipp_method(): void {}

  <<__CippGlobal>>
  public function cipp_global_method(): void {}

  <<__CippLocal('a')>>
  public function cipp_local_method(): void {}

  <<__CippRx>>
  public function cipp_rx_method(): void {}
}

<<__Cipp('b')>>
function cipp_function(): void {}

<<__CippGlobal>>
function cipp_global_function(): void {}

<<__CippLocal(C::class)>>
function cipp_local_function(): void {}

<<__CippRx>>
function cipp_rx_function(): void {}
