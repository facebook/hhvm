<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>

interface SuperBuilder {
  abstract const type TChainable as SuperBuilder;

  public function chain(): this::TChainable;
}

abstract class OuterSuper {
  abstract const type T
    as SuperBuilder
    super SuperBuilder with {
      type TChainable = this::T;
    };

  protected abstract function lower_bound(): SuperBuilder with {
    type TChainable = this::T;
  };

  public function repro(): void {
    $chainable = $this->lower_bound()->chain();
    $chainable = $chainable->chain();
    hh_expect<SuperBuilder>($chainable);
  }
}
