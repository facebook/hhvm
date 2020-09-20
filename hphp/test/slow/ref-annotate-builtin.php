<?hh

class X {
  private $a;
  private int $b;
  private arraylike $c;
  private stdclass $d;
}

function foo($value) {
  $className = get_class($value);
  $ref_obj = new ReflectionObject($value);
  $property_keys = $ref_obj->getProperties();
  var_dump(count($property_keys));
}


<<__EntryPoint>>
function main_ref_annotate_builtin() {
foo(new X);
}
