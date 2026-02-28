<?hh

// If anything breaks, it's should be easier to debug by running shell:
// #export TRACE=objprof:3

function get_instances(string $cls, ?darray $objs) :mixed{
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, vec[]), "instances", 0);
}
function get_bytes_eq(string $cls, ?darray $objs) :mixed{
  if (!$objs) return 0;
  $bytes = get_bytes($cls, $objs);
  $bytesd = get_bytesd($cls, $objs);
  if (HH\Lib\Legacy_FIXME\neq($bytes, $bytesd)) {
    echo "(BAD) Normalized bytes mismatch: ".var_export($objs, true)."\n";
  }
  return $bytes;
}
function get_bytes(string $cls, ?darray $objs) :mixed{
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, vec[]), "bytes", 0);
}
function get_bytesd(string $cls, ?darray $objs) :mixed{
  if (!$objs) return 0;
  return hphp_array_idx(hphp_array_idx($objs, $cls, vec[]),
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
class EmptyClass {}

// TEST: nullifying variables removes their tracking
class EmptyClass2 {}

// TEST: sizes of classes (including private props)
class SimpleProps { // 16+16+16 = 48
  private string $prop1 = "one"; // 16 (TypedValue bytes) = 16 (static str)
  protected int $prop2 = 2; // 16
  public bool $prop3 = true; // 16
}

// TEST: sizes of arrays
class SimpleArrays {
  public varray $arrEmpty = vec[]; // 16 (tv) static
  public darray $arrMixed = dict[ // 16 (tv) static
    "somekey" => "someval",
    321 => 3,
  ];
  public varray<int> $arrNums = vec[];
  function __construct() {
    $this->arrNums = vec[
      2012,
      2013,
      rand(1, 2)
    ]; // 32 + (16 * 3) = 80
  }
}

// TEST: sizes of dynamic props
class DynamicClass {}

// TEST: async handle
async function myAsyncFunc(): Awaitable<int> { return 42; }
class SharedStringClass {
  public ?string $val_ref = null;
  public function __construct(string $str) {
    $this->val_ref = $str; // inc 2 + inc 3
  }
}
class SharedArrayClass {
  public ?varray_or_darray $val_ref = null;
  public function __construct(varray_or_darray $arr) {
    $this->val_ref = $arr;
  }
}
class NestedArrayClass {
  public ?varray_or_darray $val_ref = null;
  public function __construct(varray_or_darray $arr) {
    $this->val_ref = $arr;
  }
}

<<__EntryPoint>>
function main_objprof() :mixed{
$myClass2 = new EmptyClass();              // ++
__hhvm_intrinsics\launder_value($myClass2);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass2);
$emptyCount = get_instances("EmptyClass", $objs);
echo $emptyCount ? "(GOOD) Tracking when enabled\n" :
     "(BAD) Not tracking when enabled: \n".var_export($objs, true)."\n";
$ObjSize = get_bytes("EmptyClass", $objs) / $emptyCount;
$objs = null;
$myClass = new EmptyClass2();              // -- ++
$myClass2 = new EmptyClass2();             // -- ++
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
$instances_before = get_instances("EmptyClass2", $objs);
echo $instances_before == 2
  ? "(GOOD) Tracking works\n"
  : "(BAD) Tracking failed: ".var_export($objs, true)."\n";
$objs = null;

$myClass = null;                           // --
$myClass2 = null;                          // --
$objs = objprof_get_data();
$instances_after = get_instances("EmptyClass2", $objs);
echo $instances_after
  ? "(BAD) Untracking failed: ".var_export($objs, true)."\n"
  : "(GOOD) Untracking works\n";
$objs = null;
$myClass = new SimpleProps();              // ++
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('SimpleProps', $objs) == $ObjSize + 16 + 16 + 16 &&
     HH\Lib\Legacy_FIXME\eq(get_bytesd('SimpleProps', $objs), 48 + $ObjSize) // String is static
  ? "(GOOD) Bytes (props) works\n"
  : "(BAD) Bytes (props) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = new SimpleArrays();
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('SimpleArrays', $objs) == $ObjSize + 80 + 16 * 2  &&
     HH\Lib\Legacy_FIXME\eq(get_bytesd('SimpleArrays', $objs), $ObjSize + 80 + 16 * 2)
  ? "(GOOD) Bytes (arrays) works\n"
  : "(BAD) Bytes (arrays) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = new DynamicClass();
$dynamic_field = 'abcd'; // 16
$dynamic_field2 = rand(1234, 1235);  // 16 (dynamic properties - always converted to static str)
$myClass->$dynamic_field = 1; // 16
$myClass->$dynamic_field2 = 1; // 16
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('DynamicClass', $objs) == $ObjSize + 16 * 4 &&
     HH\Lib\Legacy_FIXME\eq(get_bytesd('DynamicClass', $objs), $ObjSize + 16 * 4)
  ? "(GOOD) Bytes (dynamic) works\n"
  : "(BAD) Bytes (dynamic) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = myAsyncFunc();
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
$padding = $ObjSize == 12 ? 4 : 0;
echo get_bytes_eq(StaticWaitHandle::class, $objs) == 16 + $ObjSize + $padding // handle size + object size + padding
  ? "(GOOD) Bytes (Async) works\n"
  : "(BAD) Bytes (Async) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: map with int and string keys (Mixed)
$myClass = Map{};
__hhvm_intrinsics\launder_value($myClass);
$MapSize = get_bytes('HH\Map', objprof_get_data());
__hhvm_intrinsics\launder_value($myClass);
$myClass = Map {
  "abc" => 1, // 16 + 16 = 32
  1 => getStr(2), // 16 + 16 + 2 = 34
  1234123 => 3 // 16 + 16 = 32
};
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('HH\\Map', $objs) == $MapSize + 32 + 34 + 32 &&
     HH\Lib\Legacy_FIXME\eq(get_bytesd('HH\\Map', $objs), $MapSize + 32 + 34 + 32)
  ? "(GOOD) Bytes (Mixed Map) works\n"
  : "(BAD) Bytes (Mixed Map) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: vector with int and string vals (Packed)
$myClass = Vector {
  "abc", // 16 = 16
  1, // 16
};
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('HH\\Vector', $objs) == $ObjSize + 32 &&
     HH\Lib\Legacy_FIXME\eq(get_bytesd('HH\\Vector', $objs), $ObjSize + 32)
  ? "(GOOD) Bytes (Vector) works\n"
  : "(BAD) Bytes (Vector) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: set with int and string keys
$myClass = Set{};
__hhvm_intrinsics\launder_value($myClass);
$SetSize = get_bytes('HH\Set', objprof_get_data());
__hhvm_intrinsics\launder_value($myClass);
$myClass = Set {
  getStr(3), // (3 + 16) * 2 = 38
  getStr(4), // (4 + 16) * 2 = 40
};
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes('HH\\Set', $objs) == $SetSize + 78 && // SetSize + 38+40
     HH\Lib\Legacy_FIXME\eq(get_bytesd('HH\\Set', $objs), $SetSize+78 -3-4) // SetSize+ 38+40
  ? "(GOOD) Bytes (Set) works\n"
  : "(BAD) Bytes (Set) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: basic ref count
$myClass = Map {
  getStr(19) => getStr(17),
};
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes_eq('HH\\Map', $objs) == $MapSize + 19+17+16+16 // MapSize+35=109
  ? "(GOOD) Bytes (RefCount) works\n"
  : "(BAD) Bytes (RefCount) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: multiple ref counted strings
$mystr = getStr(9); // inc 1
$myClass = new SharedStringClass($mystr);
$myClass2 = new SharedStringClass($mystr);
__hhvm_intrinsics\launder_value($mystr);
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($mystr);
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
echo get_bytes('SharedStringClass', $objs) == 2 * ($ObjSize + 9 + 16)  && // 114
     HH\Lib\Legacy_FIXME\eq(get_bytesd('SharedStringClass', $objs), 2  * $ObjSize + 32 + (2 * 3))
  ? "(GOOD) Bytes (SharedString) works\n"
  : "(BAD) Bytes (SharedString) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: multiple ref counted arrays
$my_arr = dict[ // 32 + 88 = 120
  getStr(4) => getStr(8), // 4 + 8 + 16 + 16 = 44
  getStr(5) => getStr(7), // 5 + 7 + 16 + 16 = 44
];
$myClass = new SharedArrayClass($my_arr);
$myClass2 = new SharedArrayClass($my_arr);
$my_arr = null;
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
__hhvm_intrinsics\launder_value($myClass2);
echo get_bytes('SharedArrayClass', $objs) == ($ObjSize + 120) * 2  && // 152 * 2
     HH\Lib\Legacy_FIXME\eq(get_bytesd('SharedArrayClass', $objs), $ObjSize + 120 + $ObjSize + 16) //
  ? "(GOOD) Bytes (SharedArray) works\n"
  : "(BAD) Bytes (SharedArray) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = null;
$myClass2 = null;

// TEST: multiple ref counted nested arrays
$my_arr = vec[ // 32 + 76 = 108
  dict[getStr(4) => getStr(8)], // 4 + 8 + 16 + 16 = 44 + 32 = 76
];
$myClass = new NestedArrayClass($my_arr);
$my_arr = null;
__hhvm_intrinsics\launder_value($myClass);
$objs = objprof_get_data();
__hhvm_intrinsics\launder_value($myClass);
echo get_bytes_eq('NestedArrayClass', $objs) == ($ObjSize + 108)   // 140
  ? "(GOOD) Bytes (NestedArray) works\n"
  : "(BAD) Bytes (NestedArray) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = null;

// LAST TEST: Dont crash on custom types
//$xml = simplexml_load_string('<root><hello>world</hello></root>');
//$objs = objprof_get_data();
//__hhvm_intrinsics\launder_value($xml);
echo "(GOOD) Got here without crashing\n";
}
