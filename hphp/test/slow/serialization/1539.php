<?hh

class A implements Serializable {
  public $__foo = true;
  public function serialize() :mixed{
    return null;
  }
  public function unserialize($serialized) :mixed{
  }
}

 <<__EntryPoint>>
function main_1539() :mixed{
var_dump(unserialize(serialize(new A())));
}
