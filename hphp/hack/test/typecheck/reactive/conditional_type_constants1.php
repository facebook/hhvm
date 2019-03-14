<?hh // partial

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
}

abstract class ValueFactory {
  const type TLoader as IValueLoader<IValue> = IValueLoader<IValue>;
  abstract const type TValueSet as IValueSet;
  abstract const type TValueFromSet as this::TValueSet::TValue;

  public function __construct(private TLoader $loader) {}

  <<__RxShallow>>
  abstract protected static function unwrap(?IValue $x): this::TValueFromSet;

  <<__RxShallow, __OnlyRxIfImpl(IRxValueFactory::class)>>
  final public async function gen(): Awaitable<?this::TValueFromSet> {
    $result = await $this->loader->gen();
    $r = null ?? static::unwrap($result);
    return $r;
  }
}

class TLoader {
  <<__Rx>>
  public function gen() {}
}
