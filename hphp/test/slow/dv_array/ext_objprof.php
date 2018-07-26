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

function dump($data) {
  ksort(&$data);
  var_dump($data);
}

$node = new NodeClass();
$node->child = new LeafClass();
dump(objprof_get_data());
dump(objprof_get_paths());
dump(objprof_get_data(OBJPROF_FLAGS_DEFAULT, varray[LeafClass::class]));
dump(objprof_get_paths(OBJPROF_FLAGS_DEFAULT, varray[LeafClass::class]));
dump(objprof_get_strings(0));
