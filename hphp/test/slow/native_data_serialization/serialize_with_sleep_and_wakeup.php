<?hh

/**
 * This is more of a test for testing the behavior of
 * serialization/unserialization when a class with native data implements
 * __sleep() and __wakeup() methods.
 */

class Foo {
}

class Bar extends ReflectionClass {

  public $prop = 123;
  public $dontSerializeMe = "not serialized";
  private $meh = 456;

  public function __sleep() :mixed{
    var_dump("__sleep invoked");
    var_dump($this->name);
    var_dump($this->getName());
    return vec['name', 'prop', 'meh'];
  }

  public function __wakeup() :mixed{
    var_dump("__wakeup invoked");
    var_dump($this->prop);
    var_dump($this->dontSerializeMe);
    var_dump($this->name);
    var_dump($this->getName());
  }

}


<<__EntryPoint>>
function main_serialize_with_sleep_and_wakeup() :mixed{
$rc = new Bar(Foo::class);
$rc->prop = 1337;
$rc->name = 'Foo';
$rc->dontSerializeMe = "serialized";
$serialized = serialize($rc);
var_dump(json_encode($serialized));
var_dump(unserialize($serialized));
}
