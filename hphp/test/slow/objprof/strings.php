<?hh

// If anything breaks, it's should be easier to debug by running shell:
// #export TRACE=objprof:3

function get_dups(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, varray[]), "dups", 0);
}
function get_refs(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, varray[]), "refs", 0);
}
function get_srefs(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, varray[]), "srefs", 0);
}
function get_path(string $cls, ?array $objs) {
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, varray[]), "path", 0);
}

function getStr(int $len): string {
  $ret = "";
  for ($i = 0; $i < $len; ++$i) {
    $ret .= "X";
  }
  return $ret;
}
class SimpleProps {
  private string $prop1 = "one";
  protected int $prop2 = 2;
  public bool $prop3 = true;
  public ?string $prop4;
  public ?string $prop5;
  public string $prop6a;
  public string $prop6b;
}

// TEST: dynamic props
class DynamicClass {}


// TEST: simple props
<<__EntryPoint>>
function main_strings() {
$shared = getStr(4);

$myClass = new SimpleProps();
$myClass->prop4 = getStr(3);
$myClass->prop5 = getStr(3);
$myClass->prop6a = $shared;
$myClass->prop6b = $shared;

$objs = objprof_get_strings(0);
__hhvm_intrinsics\launder_value($myClass);
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
$var = 'mykey1';
$var2 = getStr(1);
$myClass = new DynamicClass();
$myClass->$var = getStr(2);
$myClass->$var2 = getStr(3);

$objs = objprof_get_strings(0);
__hhvm_intrinsics\launder_value($myClass);
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
$myClass["root"] = varray[];
$myClass["root"][] = "one";
$myClass["root"][] = "one";
$myClass["root"][$two] = getStr(2);
$objs = objprof_get_strings(0);
__hhvm_intrinsics\launder_value($myClass);
echo get_path('one', $objs) === "HH\\Map:array():[\"root\"]:array():[0]" &&
     get_path('root', $objs) === "HH\\Map:array()" &&
     get_path('XX', $objs) === "HH\\Map:array():[\"root\"]:array()" &&
     get_dups('one', $objs) === 1 &&
     get_dups('root', $objs) === 1 &&
     get_dups('XX', $objs) === 2 &&
     get_srefs('one', $objs) === 2 &&
     get_srefs('root', $objs) === 1 &&
     get_srefs('XX', $objs) === 0
  ? "(GOOD) Path (complex) works\n"
  : "(BAD) Path (complex) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: pairs
$myClass = Pair {'lol', 'whut'};
$objs = objprof_get_strings(0);
__hhvm_intrinsics\launder_value($myClass);
echo get_path('lol', $objs) === "HH\\Pair" &&
     get_path('whut', $objs) === "HH\\Pair" &&
     get_dups('lol', $objs) === 1 &&
     get_dups('whut', $objs) === 1 &&
     get_refs('lol', $objs) === 1 &&
     get_refs('whut', $objs) === 1 &&
     get_srefs('lol', $objs) === 1 &&
     get_srefs('whut', $objs) === 1
  ? "(GOOD) Pairs work\n"
  : "(BAD) Pairs failed: ".var_export($objs, true)."\n";
$objs = null;

echo "(GOOD) Got here without crashing\n";
}
