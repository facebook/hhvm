<?hh // strict

<<__ConsistentConstruct>>
abstract class Singleton<T> {
  public static function instance(): Singleton<T> {
    static $instance = new static();
    return $instance;
  }

  public static function bad_idea(): Singleton<T> {
    return self::instance();
  }

  abstract public function get(): T;
}

class IntSingleton extends Singleton<int> {
  public function get(): int {
    return 0;
  }
}

class StrSingleton extends Singleton<string> {
  public function get(): string {
    return '';
  }
}

function expects_int(int $x): void {}

function unsound(): void {
  // sets the static scope variable $instance in Singleton::instance()
  // to an instance of StrSingleton
  StrSingleton::bad_idea();

  // will invoke Singleton::instance(), returning StrSingleton, but
  // we would believe it returns Singleton<int>
  $i = IntSingleton::bad_idea();

  // Run time error would result since we will pass a string, not an int
  expects_int($i->get());
}
