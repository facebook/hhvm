<?hh // strict

interface I {
  abstract const type Tabstract;
}

abstract class P {
  abstract const type TI as I;
  const type T = this::TI::Tabstract;
}

class C extends P {
  const type TI = I;
}
