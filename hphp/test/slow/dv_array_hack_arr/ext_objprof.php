<?hh

class LeafClass {
  private string $str;
  public function __construct() {
    $this->str = (string)mt_rand();
  }
}

class NodeClass {
  public LeafClass $child;
}

function dump($data) :mixed{
  ksort(inout $data);
  var_dump($data);
}


<<__EntryPoint>>
function main_ext_objprof() :mixed{
$node = new NodeClass();
$node->child = new LeafClass();
dump(objprof_get_data());
dump(objprof_get_paths());
dump(objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[LeafClass::class]));
dump(objprof_get_paths(OBJPROF_FLAGS_DEFAULT, vec[LeafClass::class]));
__hhvm_intrinsics\launder_value($node);
}
