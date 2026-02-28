<?hh

final class Banana {}
function give_me_a_banana(Banana $_): void  {
}

final class WithConcreteTypeconst {}

final class Ummm {
  public function what_the_actual(): void {
    $_ = (
      int $a,
      int $b,
      int $c,
      int $d,
      WithConcreteTypeconst::TDoesNotExist $wtf,
      int $e,
    ) ==> {
      give_me_a_banana($wtf);
    };
  }
}
