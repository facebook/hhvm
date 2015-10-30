<?hh

class RootClass {
  public array<string, mixed> $children = array();
  <<__Memoize>>
  public static function getInstance() {
    return new RootClass();
  }
}
$r = RootClass::getInstance();

class ChildWithClosureMember {
  public $closure;
  public function doWork() {
    json_encode($this->closure);
  }
}

class ParentWithClosureTarget {
  public $someval = 20151012;
  public $somestring = "";
  public $child;

  public function __construct() {
    $this->somestring = json_encode(array(1,2,3));
    $this->child = new ChildWithClosureMember();
    $this->child->closure = $this->createClosure();
  }

  public function createClosure() {
    return function() {
      return $this->someval;
    };
  }
}
$r->children['MemoizedSingleton'] = new ParentWithClosureTarget();
$r->children['MemoizedSingleton']->child->doWork();

class ClassForSecondCapture { }

// We do some consolidation here because the behavior when testing
// is really non-deterministic. Both on order of scans and also
// whether things are in CPP or PHP (based on mode of test)
$echobuf = array();
function echo_buffer($str) {
  global $echobuf;
  $str = str_replace('cpp-stack', 'onsome-stack', $str);
  $str = str_replace('php-stack', 'onsome-stack', $str);
  $echobuf[] = $str;
}
function echo_flush() {
  global $echobuf;
  $echobuf_uniq = array_unique($echobuf);
  sort($echobuf_uniq);
  foreach ($echobuf_uniq as $str) {
    echo $str;
  }
  $echobuf = array();
}

$hg_for_closure = null;
$id_of_rootclass = null;
function showTestClasses($args) {
  global $hg_for_closure;
  global $id_of_rootclass;

  $classname = idx($args, 'class', 'no class...');
  $testclasses = array(
    ChildWithClosureMember::class => 1,
    ParentWithClosureTarget::class => 1,
    RootClass::class => 1,
    ClassForSecondCapture::class => 1,
  );
  if (idx($testclasses, $classname)) {
    echo_buffer("$classname\n");
    $same_node = heapgraph_node($hg_for_closure, $args['index']);
    $same_class = idx($same_node, 'class', 'null');
    if ($same_class != $classname) {
      echo "heapgraph_node broken: $classname != $same_class\n";
    }

    if ($classname == "RootClass") {
      $id_of_rootclass = $args['index'];
    }
  }
}

function showTestEdges($args) {
  global $hg_for_closure;
  $testedges = array(
    'ArrayKey:MemoizedSingleton' => 1,
    'Property:somestring' => 1,
    'Property:child' => 1,
    'Property:closure' => 1,
    'Property:children' => 1,
  );
  $name = $args['name'];
  if (idx($testedges, $name)) {
    echo_buffer("$name\n");
    $same_edge = heapgraph_edge($hg_for_closure, $args['index']);
    $same_name = idx($same_edge, 'name', 'null');
    if ($same_name != $name) {
      echo"heapgraph_edge broken: $name != $same_name\n";
    }
  }
}

function showAllEdges($edges) {
  foreach ($edges as $edge) {
    echo_buffer($edge['name']."\n");
  }
}

function showClass($node) {
  if (idx($node, 'class')) {
    echo_buffer($node['class']."\n");
  } else {
    echo_buffer($node['kind']."\n");
  }
}

function showEdgeAndClass($edge, $node) {
  echo_buffer($edge['name']."-->");
  showClass($node);
}

////////////////////////////////////////////////////
// Test starts here:

echo "Capturing two snapshots...\n";
// FIRST CAPTURE
$hg = heapgraph_create();
var_dump($hg);

// SECOND CAPTURE
$b = new ClassForSecondCapture();
$hg2 = heapgraph_create();
var_dump($hg2);

// STATS
echo "Stats for first capture:\n";
$stats = heapgraph_stats($hg);
var_dump($stats);

// OUT OF BOUNDS
echo "Getting invalid nodes and edges:\n";
$inv_edge = heapgraph_edge($hg, -1) ?: heapgraph_edge($hg, 999999);
$inv_node = heapgraph_node($hg, -1) ?: heapgraph_node($hg, 999999);
echo $inv_edge ?: $inv_node ?: "Invalid nodes and edges work OK.\n";
echo_flush();

// TRAVERSAL
echo "\nTraversing second capture:\n";
$hg_for_closure = $hg2;
heapgraph_foreach_node($hg2, 'showTestClasses');
echo_flush();

echo "\nTraversing first capture:\n";
$hg_for_closure = $hg;
heapgraph_foreach_node($hg, 'showTestClasses');
echo_flush();

echo "\nTraversing edges for first capture:\n";
$hg_for_closure = $hg;
heapgraph_foreach_edge($hg, 'showTestEdges');
echo_flush();

echo "\nTraversing roots for first capture:\n";
heapgraph_foreach_root($hg, 'showTestEdges');
echo_flush();

// CHILDREN / PARENTS
echo "\nGetting in edges of root class:\n";
$in_edges = heapgraph_node_in_edges($hg, $id_of_rootclass);
showAllEdges($in_edges);
echo_flush();

echo "\nGetting out edges of root class:\n";
$out_edges = heapgraph_node_out_edges($hg, $id_of_rootclass);
showAllEdges($out_edges);
echo_flush();

// DFS NODES
echo "\nDoing DFS from root class on nodes:\n";
heapgraph_dfs_nodes($hg, array($id_of_rootclass), array(), 'showClass');
echo_flush();

echo "\nDoing DFS from root class on nodes (skipping root):\n";
heapgraph_dfs_nodes(
  $hg, array($id_of_rootclass), array($id_of_rootclass), 'showClass'
);
echo_flush();
