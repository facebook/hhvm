<?hh

class MySerializable implements Serializable {
  public function __construct(public $foo)[] {}

  public function serialize() :mixed{
    return serialize($this->foo);
  }

  public function unserialize($str) :mixed{
    $this->foo = unserialize($str);
  }
}

class A {}

<<__EntryPoint>> function main(): void {
  $s = new MySerializable(new A());

  var_dump(unserialize(serialize($s)));
  var_dump(unserialize(serialize($s), dict['allowed_classes' => false]));
}
