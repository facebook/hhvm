<?hh

class A implements Serializable {
  public $__foo = true;
  public function serialize() :mixed{
    return serialize(dict['a' => 'apple', 'b' => 'banana']);
  }
  public function unserialize($serialized) :mixed{
    $props = unserialize($serialized);
    $this->a = $props['a'];
    $this->b = $props['b'];
  }
}

 <<__EntryPoint>>
function main_1540() :mixed{
$obj = unserialize(serialize(new A()));
 var_dump($obj->b);
}
