<?hh

class A implements Serializable {
  public $__foo = true;
  public function serialize() {
    return null;
  }
  public function unserialize($serialized) {
  }
}

 <<__EntryPoint>>
function main_1539() {
var_dump(unserialize(serialize(new A())));
}
