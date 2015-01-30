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
class EmptyClass {}
$myClass2 = new EmptyClass();              // ++
$objs = objprof_get_data();
$emptyCount = get_instances("EmptyClass", $objs);
echo $emptyCount ? "(GOOD) Tracking when enabled\n" :
     "(BAD) Not tracking when enabled: \n".var_export($objs, true)."\n";
$ObjSize = get_bytes("EmptyClass", $objs) / $emptyCount;
$objs = null;

// TEST: nullifying variables removes their tracking
class EmptyClass2 {}
$myClass = new EmptyClass2();              // -- ++
$myClass2 = new EmptyClass2();             // -- ++
$objs = objprof_get_data();
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

// TEST: sizes of classes (including private props)
class SimpleProps { // 19+16+16 = 51
  private string $prop1 = "one"; // 3 (byte x char) + 16 (TypedValue bytes) = 19
  protected int $prop2 = 2; // 16
  public bool $prop3 = true; // 16
}
$myClass = new SimpleProps();              // ++
$objs = objprof_get_data();
echo get_bytes('SimpleProps', $objs) == $ObjSize + 19 + 16 + 16 && // 83
     get_bytesd('SimpleProps', $objs) == 51 + $ObjSize - 3 // String is static
  ? "(GOOD) Bytes (props) works\n"
  : "(BAD) Bytes (props) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: sizes of arrays
class SimpleArrays {
  public array $arrEmpty = array(); // 16 (tv) + 16 (ArrayData) = 32
  public array $arrMixed = array( // 32 (ArrayData) + 46 + 32 = 110
    "somekey" => "someval", // 2 * (7 chars + 16 bytes object) = 46
    321 => 3, // 16 * 2 = 32
  );
  public array<int> $arrNums = array(
    2012,
    2013,
    2014
  ); // 32 + (16 * 3) = 80
}
$myClass = new SimpleArrays();
$objs = objprof_get_data();
echo get_bytes('SimpleArrays', $objs) == $ObjSize + 80 + 110 + 32 && // 254
     get_bytesd('SimpleArrays', $objs) == $ObjSize + (16 * 3) // 3 Static Arrays
  ? "(GOOD) Bytes (arrays) works\n"
  : "(BAD) Bytes (arrays) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: sizes of dynamic props
class DynamicClass {}
$myClass = new DynamicClass();
$dynamic_field = 'abcd'; // 16 + 4
$dynamic_field2 = 1234;  // 16 + 4 (dynamic properties - always string)
$myClass->$dynamic_field = 1; // 16
$myClass->$dynamic_field2 = 1; // 16
$objs = objprof_get_data();
echo get_bytes('DynamicClass', $objs) == $ObjSize + 20 + 20 + 32 && // 104
     get_bytesd('DynamicClass', $objs) == $ObjSize + 72 - 4 - 4
  ? "(GOOD) Bytes (dynamic) works\n"
  : "(BAD) Bytes (dynamic) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: async handle
async function myAsyncFunc(): Awaitable<int> { return 42; }
$myClass = myAsyncFunc();
$objs = objprof_get_data();
echo get_bytes_eq(StaticWaitHandle::class, $objs) == 16 + $ObjSize // handle size
  ? "(GOOD) Bytes (Async) works\n"
  : "(BAD) Bytes (Async) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: map with int and string keys (Mixed)
$myClass = Map{};
$MapSize = get_bytes('HH\Map', objprof_get_data());
$myClass = Map {
  "abc" => 1, // 3 + 16 + 16 = 35
  1 => "22", // 16 + 16 + 2 = 34
  1234123 => 3 // 16 + 16 = 32
};
$objs = objprof_get_data();
echo get_bytes('HH\\Map', $objs) == $MapSize + 32 + 34 + 35 && // MapSize+101
     get_bytesd('HH\\Map', $objs) == $MapSize+101 - 2 - 3 // Static strings
  ? "(GOOD) Bytes (Mixed Map) works\n"
  : "(BAD) Bytes (Mixed Map) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: vector with int and string vals (Packed)
$myClass = Vector {
  "abc", // 3 + 16 = 19
  1, // 16
};
$objs = objprof_get_data();
echo get_bytes('HH\\Vector', $objs) == $ObjSize + 32 + 19 + 16 && // Vec+35=99
     get_bytesd('HH\\Vector', $objs) == $ObjSize + 67 - 3 // Static strings
  ? "(GOOD) Bytes (Vector) works\n"
  : "(BAD) Bytes (Vector) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: set with int and string keys
$myClass = Set{};
$SetSize = get_bytes('HH\Set', objprof_get_data());
$myClass = Set {
  getStr(3), // (3 + 16) * 2 = 38
  getStr(4), // (4 + 16) * 2 = 40
};
$objs = objprof_get_data();
echo get_bytes('HH\\Set', $objs) == $SetSize + 78 && // SetSize + 38+40
     get_bytesd('HH\\Set', $objs) == $SetSize+78 -3-4 // SetSize+ 38+40
  ? "(GOOD) Bytes (Set) works\n"
  : "(BAD) Bytes (Set) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: basic ref count
$myClass = Map {
  getStr(19) => getStr(17),
};
$objs = objprof_get_data();
echo get_bytes_eq('HH\\Map', $objs) == $MapSize + 19+17+16+16 // MapSize+35=109
  ? "(GOOD) Bytes (RefCount) works\n"
  : "(BAD) Bytes (RefCount) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: multiple ref counted strings
$mystr = getStr(9); // inc 1
class SharedStringClass {
  public string $val_ref = null;
  public function __construct(string $str) {
    $this->val_ref = $str; // inc 2 + inc 3
  }
}
$myClass = new SharedStringClass($mystr);
$myClass2 = new SharedStringClass($mystr);
$objs = objprof_get_data();
echo get_bytes('SharedStringClass', $objs) == 2 * ($ObjSize + 9 + 16)  && // 114
     get_bytesd('SharedStringClass', $objs) == 2  * $ObjSize + 32 + (2 * 3)
  ? "(GOOD) Bytes (SharedString) works\n"
  : "(BAD) Bytes (SharedString) failed: ".var_export($objs, true)."\n";
$objs = null;

// TEST: multiple ref counted arrays
$my_arr = array( // 32 + 88 = 120
  getStr(4) => getStr(8), // 4 + 8 + 16 + 16 = 44
  getStr(5) => getStr(7), // 5 + 7 + 16 + 16 = 44
);
class SharedArrayClass {
  public array $val_ref = null;
  public function __construct(array $arr) {
    $this->val_ref = $arr;
  }
}
$myClass = new SharedArrayClass($my_arr);
$myClass2 = new SharedArrayClass($my_arr);
$my_arr = null;
$objs = objprof_get_data();
echo get_bytes('SharedArrayClass', $objs) == ($ObjSize + 120) * 2  && // 152 * 2
     get_bytesd('SharedArrayClass', $objs) == $ObjSize + 120 + $ObjSize + 16 //
  ? "(GOOD) Bytes (SharedArray) works\n"
  : "(BAD) Bytes (SharedArray) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = null;
$myClass2 = null;

// TEST: multiple ref counted nested arrays
$my_arr = array( // 32 + 76 = 108
  array(getStr(4) => getStr(8)), // 4 + 8 + 16 + 16 = 44 + 32 = 76
);
class NestedArrayClass {
  public array $val_ref = null;
  public function __construct(array $arr) {
    $this->val_ref = $arr;
  }
}
$myClass = new NestedArrayClass($my_arr);
$my_arr = null;
$objs = objprof_get_data();
echo get_bytes_eq('NestedArrayClass', $objs) == ($ObjSize + 108)   // 140
  ? "(GOOD) Bytes (NestedArray) works\n"
  : "(BAD) Bytes (NestedArray) failed: ".var_export($objs, true)."\n";
$objs = null;
$myClass = null;

// LAST TEST: Dont crash on custom types
//$xml = simplexml_load_string('<root><hello>world</hello></root>');
//$objs = objprof_get_data();
echo "(GOOD) Got here without crashing\n";
