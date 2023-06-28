<?hh

trait Singleton {
  protected static $instances=darray[];
  abstract protected function __construct($config);
  public static function getInstance($config) :mixed{
    if (!isset(self::$instances[$serialize = serialize($config)])) {
      self::$instances[$serialize] = new self($config);
    }
    return self::$instances[$serialize];
  }
}

class MyHelloWorld {
  use Singleton;
  public function __construct($config)
  {
    var_dump( $config);
  }
}

<<__EntryPoint>> function main(): void {
$o= MyHelloWorld::getInstance(1);
$o= MyHelloWorld::getInstance(1);
$o= MyHelloWorld::getInstance(2);
$o= MyHelloWorld::getInstance(darray[1=>2]);
$o= MyHelloWorld::getInstance(darray[1=>2]);
}
