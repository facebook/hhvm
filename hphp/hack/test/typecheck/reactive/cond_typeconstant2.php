<?hh
interface Base {}
interface ILoader<T as Base> {}

interface IRxFactory {

  require extends Factory;

  abstract const type TLoader as RxLoader<Base>;


  public static function __getLoader(): RxLoader<Base>;
}

interface RxLoader<T as Base> extends ILoader<T> {
}

abstract class Factory {
  const type TLoader as ILoader<Base> = ILoader<Base>;

  <<__LSB>>
  private static ?this::TLoader $getLoader = null;


  final public static function get(): this::TLoader {
    if (Rx\IS_ENABLED) {
      return static::__getLoader();
    } else {
      if (static::$getLoader === null) {
        static::$getLoader = static::__getLoader();
      }
      return static::$getLoader;
    }
  }

  public static function __getLoader(): this::TLoader {
    throw new Exception("");
  }
}
