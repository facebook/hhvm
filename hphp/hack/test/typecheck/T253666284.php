<?hh

interface IDontCareAboutTB {}

abstract class DefinesTB implements IDontCareAboutTB {
  abstract const type TB;
}

abstract class UsesTB {
  abstract const type TA as DefinesTB;
  const type TB =
    this::TA::TB; // Requires TB to be defined in the upper bound of TA
  public function __construct(protected this::TB $tb): void {}
}

trait TRedeclareTA {
  abstract const type TA as IDontCareAboutTB; // No requirement on TB so we're good
}

final class Banana {}

function giveMeABanana(Banana $_): void {}

trait TYolo {
  require extends UsesTB;
  use TRedeclareTA;
  public function behold(): void {
    // This is Tany because in the body of the trait, TA has an upperbound of IDontCareAboutTB
    $i_do_not_respect_lattices = $this->tb;
    // Catchable fatal error: Argument 1 passed to giveMeABanana() must be an instance of Banana, int given
    giveMeABanana($i_do_not_respect_lattices);
  }
}

final class ConcreteDefinesTB extends DefinesTB {
  const type TB = int;
}

class CYolo extends UsesTB {
  const type TA = ConcreteDefinesTB;
  use TYolo;
}

<<__EntryPoint>>
function letsGo(): void {
  $x = new CYolo(42);
  $x->behold();
}
