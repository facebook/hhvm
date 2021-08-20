<?hh

abstract class Effectful {
  abstract const ctx C;
}
class PureEffectful extends Effectful {
  const ctx C = [];
}
class WritePropsEffectful extends Effectful {
  const ctx C = [write_props];
}


abstract class Pointer {
  abstract const type Tpointer as Effectful;
}
class PurePointer extends Pointer {
  const type Tpointer = PureEffectful;
}
class WritePropsPointer extends Pointer {
  const type Tpointer = WritePropsEffectful;
}

function f<reify T as Pointer>()[T::Tpointer::C]: void {}

function test()[]: void {
  f<PurePointer>(); // ok

  f<WritePropsPointer>(); // error
}
