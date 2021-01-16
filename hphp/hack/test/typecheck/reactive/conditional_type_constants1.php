<?hh
interface IValue {
}

interface IValueLoader<+TValue as IValue> {
  <<__Rx, __OnlyRxIfImpl(IRxValueLoader::class)>>
  public function gen(): Awaitable<?TValue>;
}

interface IRxValueLoader<+TValue as IValue> extends IValueLoader<TValue> {
  <<__Rx, __Override>>
  public function gen(): Awaitable<?TValue>;
}

interface IValueSet {
  abstract const type TValue as IValue;
}

interface IRxValueFactory {
  require extends ValueFactory;

  abstract const type TLoader as IRxValueLoader<IValue>;

  <<__RxShallow>>
  public function gen(): Awaitable<?this::TValueFromSet>;
}

abstract class ValueFactory {
  const type TLoader as IValueLoader<IValue> = IValueLoader<IValue>;
  abstract const type TValueSet as IValueSet;
  abstract const type TValueFromSet as this::TValueSet::TValue;

  public function __construct(private this::TLoader $loader) {}

  <<__RxShallow>>
  abstract protected static function unwrap(?IValue $x): this::TValueFromSet;

  <<__RxShallow, __OnlyRxIfImpl(IRxValueFactory::class)>>
  final public async function gen(): Awaitable<?this::TValueFromSet> {
    $result = await $this->loader->gen(); // FIXME(coeffects) should type-check
    $r = null ?? static::unwrap($result);
    return $r;
  }
}
