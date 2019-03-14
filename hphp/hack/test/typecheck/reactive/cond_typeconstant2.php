<?hh // partial
interface Base {}
interface ILoader<T as Base> {}

interface IRxFactory {

  require extends Factory;

  abstract const type TLoader as RxLoader<Base>;

  <<__RxLocal>>
  public static function __getLoader(): RxLoader<Base>;
}

interface RxLoader<T as Base> extends ILoader<T> {
}

abstract class Factory {
  const type TLoader as ILoader<Base> = ILoader<Base>;

  <<__LSB>>
  private static $getLoader = null;

  <<__RxShallow, __OnlyRxIfImpl(IRxFactory::class)>>
  final public static function get(): this::TLoader {
    if (HH\Rx\IS_ENABLED) {
      return static::__getLoader();
    } else {
      if (static::$getLoader === null) {
        static::$getLoader = static::__getLoader();
      }
      return static::$getLoader;
    }
  }
  <<__RxLocal, __OnlyRxIfImpl(IRxFactory::class)>>
  public static function __getLoader(): this::TLoader {
    throw new Exception("");
  }
}
