<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

interface IFields<T> {}

interface IBase extends IFields<this::TEnt> {
  abstract const type TEnt as IBase super this;
  public function getEnt(): this::TEnt;
}

function wants<
  TEnt as T super T,
  T as IBase with { type TEnt = TEnt } super IBase with { type TEnt = TEnt },
>(IFields<T> $_): void {}

function refit<
  T as IBase with { type TEnt = T } super IBase with { type TEnt = T },
>(T $value): void {
  $via_meth_caller = meth_caller(IBase::class, 'getEnt')($value);
  $via_direct_call = $value->getEnt();

  hh_expect<T>($via_meth_caller);
  hh_expect<T>($via_direct_call);
  wants($via_meth_caller);
  wants($via_direct_call);
}
