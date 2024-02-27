<?hh

interface IWrappedValue {
  abstract const type TInnerValue;
}

class WrappedIntValue implements IWrappedValue {
  const type TInnerValue = int;
}

class SomeClass<TWrappedValue as IWrappedValue> {
  public function getValue<
    TGetWrappedValue as IWrappedValue with { type TInnerValue = TInnerValue },
    TInnerValue,
  >(): TInnerValue where TGetWrappedValue = TWrappedValue { return 42; }
}

function myTest() {
  $instance = new SomeClass<WrappedIntValue>();
  $value = $instance->AUTO332
}
