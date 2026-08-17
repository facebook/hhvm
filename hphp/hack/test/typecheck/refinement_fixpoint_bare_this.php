<?hh

abstract class Banana {}

function take_a_banana(Banana $_): void {}

abstract class Ok {
  abstract const type TChainable as Ok with {
    type TChainable = this::TChainable;
  };

  public abstract function chain(): this::TChainable;

  public function repro(): void {
    $chainable = $this->chain();
    $not_a_banana = $chainable->chain();
    take_a_banana($not_a_banana);
  }
}
