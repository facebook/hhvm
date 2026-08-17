<?hh

abstract class Banana {}

function take_a_banana(Banana $_): void {}

interface IBuilder {
  abstract const type TChainable as IBuilder;

  public function chain(): this::TChainable;
}

abstract class Repro {
  abstract const type T as IBuilder with {
    type TChainable = this::T;
  };

  protected abstract function builder(): this::T;

  public function repro(): void {
    $builder = $this->builder();
    $chainable = $builder->chain();
    $not_a_banana = $chainable->chain();
    take_a_banana($not_a_banana);
  }
}
