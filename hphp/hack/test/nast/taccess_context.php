<?hh

class WithConstant {
  const ctx C = [io];
  public function uses_self_C()[self::C]: void {}
}

abstract class AnotherClassWithConstant {
  abstract const ctx C;

  abstract public function uses_this_C()[this::C]: void;

  abstract public function uses_another_C()[WithConstant::C]: void;
}
