<?hh // strict

interface I {
  <<__Enforceable>>
  abstract const type T as num;
}

interface J extends I {
  abstract const type T as int;
}

abstract class C {

  abstract const type TDataInterface as J;
  const type TDdata = this::TDataInterface::T;

  public function test(mixed $data): this::TDdata {
    return $data as this::TDdata;
  }
}
