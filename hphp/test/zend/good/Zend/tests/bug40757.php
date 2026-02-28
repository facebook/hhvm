<?hh
class Base {
  private $p1='sadf';

  function getFields($obj):mixed{
    return get_object_vars($obj);
  }
}

class Child extends Base { }
<<__EntryPoint>> function main(): void {
$base=new Base();
print_r($base->getFields(new Base()));
$child=new Child();
print_r($child->getFields(new Base()));
}
