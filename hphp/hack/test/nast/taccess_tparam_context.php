<?hh

abstract class WithConstant {
  abstract const ctx C;
}

class C<reify T as WithConstant> {
  public function taccess_tparam_context()[T::C]: void {}
}
