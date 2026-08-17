<?hh

interface UpperBuilder {
  abstract const type TChainable as UpperBuilder;

  public function chain(): this::TChainable;
}

abstract class LooseUpper {
  abstract const type T as UpperBuilder with {
    type TChainable as this::T;
  };

  protected abstract function builder(): this::T;

  public function repro(): void {
    $chainable = $this->builder()->chain();
    $chainable = $chainable->chain();
    hh_expect<UpperBuilder>($chainable);
  }
}

interface LowerBuilder {
  abstract const type TChainable as LowerBuilder;

  public function chain(): this::TChainable;
}

abstract class LooseLower {
  abstract const type T as LowerBuilder with {
    type TChainable super this::T;
  };

  protected abstract function builder(): this::T;

  public function repro(): void {
    $chainable = $this->builder()->chain();
    $chainable = $chainable->chain();
    hh_expect<LowerBuilder>($chainable);
  }
}
