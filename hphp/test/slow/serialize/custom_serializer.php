<?hh

class MySerializable implements Serializable {
  public function __construct(public $foo) {}

  public function serialize() {
    return serialize($this->foo);
  }

  public function unserialize($str) {
    $this->foo = unserialize($str);
  }
}

class A {}

<<__EntryPoint>> function main(): void {
  $s = new MySerializable(new A());

  var_dump(unserialize(serialize($s)));
  var_dump(unserialize(serialize($s), darray['allowed_classes' => false]));
}
