<?hh

interface ILoader<T> {
  public static function gen(): Awaitable<T>;

  public function set(T $x);
}

abstract class Loader implements ILoader<this::TLoadsType> {
  abstract const type TLoadsType;

  public function set(this::TLoadsType $x): void {}
}

<<__ConsistentConstruct>>
abstract class SelfLoader extends Loader {
  const type TLoadsType = this;

  public static async function gen(): Awaitable<this::TLoadsType> {
    return new static();
  }
}

class ALoader extends SelfLoader {}

class BLoader extends ALoader {}

class OtherALoader extends Loader {
  const type TLoadsType = ALoader;

  public static async function gen(): Awaitable<ALoader> {
    return new BLoader();
  }
}

async function test(): Awaitable<void> {
  concurrent {
    $a = await ALoader::gen();
    $b = await BLoader::gen();
    $o = await OtherALoader::gen();
  }

  $a->set($o);
}
