<?hh

class X {
  private $a;
  private int $b;
  private array $c;
  private stdclass $d;
}

function foo($value) {
  $className = get_class($value);
  $ref_obj = new ReflectionObject($value);
  $property_keys = $ref_obj->getProperties();
  var_dump(count($property_keys));
}

foo(new X);
