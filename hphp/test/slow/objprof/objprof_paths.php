<?hh

// If anything breaks, it's should be easier to debug by running shell:
// #export TRACE=objprof:3

function get_instances(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "instances", 0);
}
function get_bytes_eq(string $cls, ?array $objs) {
  if (!$objs) return 0;
  $bytes = get_bytes($cls, $objs);
  $bytesd = get_bytesd($cls, $objs);
  if ($bytes != $bytesd) {
    echo "(BAD) Normalized bytes mismatch: ".var_export($objs, true)."\n";
  }
  return $bytes;
}
function get_bytes(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "bytes", 0);
}
function get_bytesd(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()),
    "bytes_normalized", 0);
}
function getStr(int $len): string {
  $ret = "";
  for ($i = 0; $i < $len; ++$i) {
    $ret .= "X";
  }
  return $ret;
}

// TEST: tracking works when enabled and not when disabled
class StaticClass {
  public static ?mixed $myStaticRef;
  public static array<mixed> $myStaticArr = array();
}

class RootClass {
  public mixed $someObject = null;
}

class NestedClass {
  public bool $nestedBool = true;
}

class ClassReachableByStaticOnly {
  public bool $someBool = true;
}

class ParentClass {
  public int $parentInt = 1;
  public NestedClass $parentNested;
}

class ChildClass extends ParentClass{
  public string $childString = "abcdef";
  public NestedClass $childNested;
  public array $mixedArr = array();
  public array $packedArr = array();
}

$childClass = new ChildClass();
$childClass->parentNested = new NestedClass();
$childClass->childNested = new NestedClass();
$childClass->mixed["MyStringKey"] = $childClass->parentNested;
$childClass->mixed[123] = $childClass->parentNested;
$childClass->packed[] = $childClass->parentNested;

$childClass2 = new ChildClass();
$childClass2->parentNested = new NestedClass();

$rootObject = new RootClass();
$rootObject->someObject = $childClass;

StaticClass::$myStaticRef = new ClassReachableByStaticOnly();
StaticClass::$myStaticArr[] = StaticClass::$myStaticRef;
$allobjs = objprof_get_paths();
$objs = array();
$objs['RootClass'] = $allobjs['RootClass'];
$objs['NestedClass'] = $allobjs['NestedClass'];
$objs['ClassReachableByStaticOnly'] = $allobjs['ClassReachableByStaticOnly'];
$objs['ChildClass'] = $allobjs['ChildClass'];
$pathstrs = array();
foreach ($objs as $name => $metrics) {
  foreach ($metrics['paths'] as $path => $path_metrics) {
    $pathstrs[] = $path.' '.idx($path_metrics,'refs',-1);
  }
}
sort($pathstrs);
echo implode("\n", $pathstrs)."\n";
