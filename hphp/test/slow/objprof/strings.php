<?hh

// If anything breaks, it's should be easier to debug by running shell:
// #export TRACE=objprof:3

function get_dups(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "dups", 0);
}
function get_refs(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "refs", 0);
}
function get_srefs(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "srefs", 0);
}
function get_path(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, array()), "path", 0);
}

function getStr(int $len): string {
  $ret = "";
  for ($i = 0; $i < $len; ++$i) {
    $ret .= "X";
  }
  return $ret;
}

// TEST: simple props
$shared = getStr(4);
class SimpleProps {
  private string $prop1 = "one";
  protected int $prop2 = 2;
  public bool $prop3 = true;
  public ?string $prop4;
  public ?string $prop5;
  public string $prop6a;
  public string $prop6b;
}

$myClass = new SimpleProps();
$myClass->prop4 = getStr(3);
$myClass->prop5 = getStr(3);
$myClass->prop6a = $shared;
$myClass->prop6b = $shared;

$objs = objprof_get_strings(0);
echo get_srefs('one', $objs) === 1 &&
     get_refs('one', $objs) === 1 &&
     get_dups('one', $objs) === 1 &&
     get_dups('XXX', $objs) === 2 &&
     get_refs('XXX', $objs) === 2 &&
     get_srefs('XXX', $objs) === 0 &&
     get_dups('XXXX', $objs) === 1 &&
     get_refs('XXXX', $objs) === 2 &&
     get_srefs('XXXX', $objs) === 0 &&
     get_path('XXXX', $objs) === 'SimpleProps:prop6a'
  ? "(GOOD) Agg (props) works\n"
  : "(BAD) Agg (props) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: dynamic props
class DynamicClass {}
$var = 'mykey1';
$var2 = getStr(1);
$myClass = new DynamicClass();
$myClass->$var = getStr(2);
$myClass->$var2 = getStr(3);

$objs = objprof_get_strings(0);
echo get_path('mykey1', $objs) === "DynamicClass" &&
     get_path('X', $objs) === "DynamicClass" &&
     get_path('XX', $objs) === "DynamicClass:[\"mykey1\"]" &&
     get_path('XXX', $objs) === "DynamicClass:[\"X\"]"
  ? "(GOOD) Path (dynamic) works\n"
  : "(BAD) Path (dynamic) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: Complex path
$myClass = Map {};
$two = getStr(2);
$myClass["root"] = array();
$myClass["root"][] = "one";
$myClass["root"][] = "one";
$myClass["root"][$two] = getStr(2);
$objs = objprof_get_strings(0);
echo get_path('one', $objs) === "HH\\Map:[\"root\"]:array():[0]" &&
     get_path('root', $objs) === "HH\\Map" &&
     get_path('XX', $objs) === "HH\\Map:[\"root\"]:array()" &&
     get_dups('one', $objs) === 1 &&
     get_dups('root', $objs) === 1 &&
     get_dups('XX', $objs) === 2 &&
     get_srefs('one', $objs) === 2 &&
     get_srefs('root', $objs) === 1 &&
     get_srefs('XX', $objs) === 0
  ? "(GOOD) Path (complex) works\n"
  : "(BAD) Path (complex) failed: ".var_export($objs, true)."\n";
echo "(GOOD) Got here without crashing\n";
