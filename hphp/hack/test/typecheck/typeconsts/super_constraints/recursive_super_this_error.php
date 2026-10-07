<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

interface IFields<T> {}

interface IBase extends IFields<this::TEnt> {
  abstract const type TEnt as IBase super this;
  public function getEnt(): this::TEnt;
}

function needs_base_fields(IFields<IBase> $_): void {}

function test(IBase $value): void {
  needs_base_fields($value->getEnt());
}
