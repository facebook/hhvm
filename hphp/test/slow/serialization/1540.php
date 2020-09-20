<?hh

class A implements Serializable {
  public $__foo = true;
  public function serialize() {
    return serialize(darray['a' => 'apple', 'b' => 'banana']);
  }
  public function unserialize($serialized) {
    $props = unserialize($serialized);
    $this->a = $props['a'];
    $this->b = $props['b'];
  }
}

 <<__EntryPoint>>
function main_1540() {
$obj = unserialize(serialize(new A()));
 var_dump($obj->b);
}
