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

class DeepClass {
  public ?DeepClass $next = null;
  public int $depth = 0;

  public static function create(int $depth): DeepClass {
    $root = new DeepClass();
    $root->depth = 0;
    $current = $root;
    for ($i = 1; $i < $depth; $i++) {
      $current->next = new DeepClass();
      $current->next->depth = $i;
      $current = $current->next;
    }
    return $root;
  }
}

class WideClass {
  public vec<LeafClass> $children = vec[];

  public static function create(int $count): WideClass {
    $obj = new WideClass();
    for ($i = 0; $i < $count; $i++) {
      $obj->children[] = new LeafClass();
    }
    return $obj;
  }
}

function dump($data) :mixed{
  ksort(inout $data);
  var_dump($data);
}

function test_basic() :mixed{
  echo "=== Basic Tests ===\n";
  $node = new NodeClass();
  $node->child = new LeafClass();
  dump(objprof_get_data());
  dump(objprof_get_paths());
  dump(objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[LeafClass::class]));
  dump(objprof_get_paths(OBJPROF_FLAGS_DEFAULT, vec[LeafClass::class]));
  __hhvm_intrinsics\launder_value($node);
}

function test_max_depth() :mixed{
  echo "\n=== Max Depth Tests ===\n";

  // Create a deep chain: depth 10
  $deep = DeepClass::create(10);

  echo "--- Unlimited depth (0) ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0);
  echo "DeepClass instances: " . ($result['DeepClass']['instances'] ?? 0) . "\n";

  echo "--- Max depth 5 ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 5);
  echo "DeepClass instances: " . ($result['DeepClass']['instances'] ?? 0) . "\n";

  echo "--- Max depth 1 ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 1);
  echo "DeepClass instances: " . ($result['DeepClass']['instances'] ?? 0) . "\n";

  __hhvm_intrinsics\launder_value($deep);
}

function test_max_visits() :mixed{
  echo "\n=== Max Visits Tests ===\n";

  // Create an object with many children
  $wide = WideClass::create(20);

  echo "--- Unlimited visits (0) ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0, 0);
  echo "WideClass instances: " . ($result['WideClass']['instances'] ?? 0) . "\n";
  echo "LeafClass instances: " . ($result['LeafClass']['instances'] ?? 0) . "\n";

  echo "--- Max visits 10 ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0, 10);
  echo "WideClass instances: " . ($result['WideClass']['instances'] ?? 0) . "\n";
  echo "LeafClass instances: " . ($result['LeafClass']['instances'] ?? 0) . "\n";

  echo "--- Max visits 5 ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 0, 5);
  echo "WideClass instances: " . ($result['WideClass']['instances'] ?? 0) . "\n";
  echo "LeafClass instances: " . ($result['LeafClass']['instances'] ?? 0) . "\n";

  __hhvm_intrinsics\launder_value($wide);
}

function test_combined_limits() :mixed{
  echo "\n=== Combined Limits Tests ===\n";

  $deep = DeepClass::create(10);

  echo "--- Both depth=3 and visits=5 ---\n";
  $result = objprof_get_data(OBJPROF_FLAGS_DEFAULT, vec[], 3, 5);
  echo "DeepClass instances: " . ($result['DeepClass']['instances'] ?? 0) . "\n";

  __hhvm_intrinsics\launder_value($deep);
}

function test_extended_functions() :mixed{
  echo "\n=== Extended Function Tests ===\n";

  $wide = WideClass::create(15);

  echo "--- objprof_get_data_extended with max_visits=8 ---\n";
  $result = objprof_get_data_extended(OBJPROF_FLAGS_DEFAULT, vec[], 0, 8);
  echo "WideClass instances: " . ($result['WideClass']['instances'] ?? 0) . "\n";
  echo "LeafClass instances: " . ($result['LeafClass']['instances'] ?? 0) . "\n";

  echo "--- objprof_get_paths with max_depth=2 ---\n";
  $result = objprof_get_paths(OBJPROF_FLAGS_DEFAULT, vec[], 2, 0);
  echo "WideClass instances: " . ($result['WideClass']['instances'] ?? 0) . "\n";
  echo "LeafClass instances: " . ($result['LeafClass']['instances'] ?? 0) . "\n";

  __hhvm_intrinsics\launder_value($wide);
}

<<__EntryPoint>>
function main_ext_objprof() :mixed{
  test_basic();
  test_max_depth();
  test_max_visits();
  test_combined_limits();
  test_extended_functions();
}
