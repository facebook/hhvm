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
    echo "(BAD) Normalized bytes mismatch: " . var_export($objs, true) . "\n";
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


// TEST: nullifying variables removes their tracking
class EmptyClass2 {}


// TEST: sizes of classes (including private props)
class SimpleProps { // 51:48
  private string $prop1 = "one"; // 19:16
  protected int $prop2 = 2; // 16:16
  public bool $prop3 = true; // 16:16
}


// TEST: sizes of arrays
class SimpleArrays {
  public array $arrEmpty = array(); // 16 (tv) + 16 (ArrayData) = 32
  public array $arrMixed = array( // 16 (tv) + 16 (ArrayData) + 46 + 32 = 110
    "somekey" => "someval", // 2 * (23:16) = 46:32
    321 => 3, // 2 * (16:16) = 32:32
  );
  public array<int> $arrNums = array(
    2012, // 16:16
    2013, // 16:16
    2014 // 16:16
  ); // 16 (tv) + 16 (ArrayData) + (16 * 3) = 80
}


// TEST: sizes of dynamic props
class DynamicClass {}


// TEST: async handle
async function myAsyncFunc(): Awaitable<int> { return 42; }
class SharedStringClass {
  public string $val_ref = null;
  public function __construct(string $str) {
    $this->val_ref = $str; // inc 2 + inc 3
  }
}
class SharedArrayClass {
  public array $val_ref = null;
  public function __construct(array $arr) {
    $this->val_ref = $arr;
  }
}
class NestedArrayClass {
  public array $val_ref = null;
  public function __construct(array $arr) {
    $this->val_ref = $arr;
  }
}


// TEST: ref counted Maps
class SimpleMapClass { // size = 2*(24(SimpleMapClass)+122) = 292
                   // sized = 2*(24(SimpleMapClass)+116) = 280
  public Map<string,mixed> $map;
  public function __construct() {
    $this->map = Map{ // size = 16(tv)+$MapSize+39+43 = 122
      'foo' => getStr(4), // size = 19+20 = 39
                          // sized = 16+20 = 36
      'bar' => getStr(8), // size = 19+24 = 43
                          // sized = 16+24 = 40
    };
  }
}
class SharedMapClass { // size = 2*(24(SharedMapClass)+122) = 292
                        // sized = 2*(24(SharedMapClass)+66) = 180
  public Map<string,mixed> $map;
  public function __construct(Map<string, mixed> $m) {
    $this->map = $m;
  }
}


// TEST: back edge
class SimpleMapClassWithBackEdge { // size = 24(Objsize)+114 = 138
                                   // sized = 24(Objsize)+108 = 132
  public Map<string,mixed> $map;
  public function __construct() {
    $this->map = Map{ // size = 16(tv)+$MapSize+39+35 = 114
                      // sized = 16(tv)+$MapSize+36+32 = 108
      'bar' => getStr(4), // size = 19+20 = 39
                          // sized = 16+20 = 36
    };
  }
}


// TEST: Validate that memory for an excluded class is correctly attributed
// to the parent root node, both in DEFAULT and USER_TYPES_ONLY modes
class ExlcudeClass {}
class SimpleClassForExclude {
  public Map<string,mixed> $map;
  public ExlcudeClass $fooCls1;
  public ExlcudeClass $fooCls2;
  public function __construct() {
    $this->map = Map{
      'foo' => getStr(4),
      'bar' => getStr(8),
    };
    $this->fooCls1 = new ExlcudeClass();
    $this->fooCls2 = new ExlcudeClass();
  }
}

<<__EntryPoint>>
function main_objprof_user_types() {
$myClass2 = new EmptyClass();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
$emptyCount = get_instances("EmptyClass", $objs);
echo $emptyCount ?
  "(GOOD) Tracking when enabled\n" :
  "(BAD) Not tracking when enabled: \n" . var_export($objs, true) . "\n";
$ObjSize = get_bytes("EmptyClass", $objs) / $emptyCount;
$objs = null;
$myClass2 = null;
$myClass = new EmptyClass2();
$myClass2 = new EmptyClass2();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
$instances_before = get_instances("EmptyClass2", $objs);
echo $instances_before == 2 ?
  "(GOOD) Tracking works\n" :
  "(BAD) Tracking failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass2 = null;
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
$instances_after = get_instances("EmptyClass2", $objs);
echo $instances_after ?
  "(BAD) Untracking failed: " . var_export($objs, true) . "\n" :
  "(GOOD) Untracking works\n";
$objs = null;
$myClass = null;
$myClass2 = null;
$myClass = new SimpleProps();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SimpleProps', $objs) == $ObjSize + 51 &&
     get_bytesd('SimpleProps', $objs) == $ObjSize + 48 ?
      "(GOOD) Bytes (props) works\n" :
      "(BAD) Bytes (props) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass = new SimpleArrays();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SimpleArrays', $objs) == $ObjSize + 80 + 110 + 32 &&
     get_bytesd('SimpleArrays', $objs) == $ObjSize + (16 * 3) ?
      "(GOOD) Bytes (arrays) works\n" :
      "(BAD) Bytes (arrays) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass = new DynamicClass();
$dynamic_field = 'abcd'; // 20:16
$dynamic_field2 = 1234;  // 20:16 (dynamic properties - always string)
$myClass->$dynamic_field = 1; // 16:16
$myClass->$dynamic_field2 = 1; // 16:16
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('DynamicClass', $objs) == $ObjSize + 20 + 20 + 16 + 16 &&
     get_bytesd('DynamicClass', $objs) == $ObjSize + (16 * 4) ?
      "(GOOD) Bytes (dynamic) works\n" :
      "(BAD) Bytes (dynamic) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass = myAsyncFunc();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq(StaticWaitHandle::class, $objs) == 0 ? // not a user node
  "(GOOD) Bytes (Async) works\n" :
  "(BAD) Bytes (Async) failed: " . var_export($objs, true) . "\n";
$objs = null;


$myClass = Map{};
$MapSize = get_bytes('HH\Map', objprof_get_data(OBJPROF_FLAGS_DEFAULT));

// TEST: map with int and string keys (Mixed)
$myClass = Map {
  "abc" => 1, // 3 + 16 + 16 = 35
  1 => "22", // 16 + 16 + 2 = 34
  1234123 => 3 // 16 + 16 = 32
};
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq('HH\\Map', $objs) == 0 ? // not a user node
  "(GOOD) Bytes (Mixed Map) works\n" :
  "(BAD) Bytes (Mixed Map) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;


// TEST: vector with int and string vals (Packed)
$myClass = Vector {
  "abc", // 19:16
  1, // 16:16
};
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq('HH\\Vector', $objs) == 0 ? // not a user node
  "(GOOD) Bytes (Vector) works\n" :
  "(BAD) Bytes (Vector) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;


// TEST: set with int and string keys
$myClass = Set{};
$SetSize = get_bytes('HH\Set', objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY));
$myClass = Set {
  getStr(3), // (3 + 16) * 2 = 38
  getStr(4), // (4 + 16) * 2 = 40
};
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq('HH\\Set', $objs) == 0 ? // not a user node
  "(GOOD) Bytes (Set) works\n" :
  "(BAD) Bytes (Set) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;


// TEST: basic ref count
$myClass = Map {
  getStr(19) => getStr(17),
};
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq('HH\\Map', $objs) == 0 ? // not a user node
  "(GOOD) Bytes (RefCount) works\n" :
  "(BAD) Bytes (RefCount) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;


// TEST: multiple ref counted strings
$mystr = getStr(9); // inc 1, 25:16
$myClass = new SharedStringClass($mystr);
$myClass2 = new SharedStringClass($mystr);
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SharedStringClass', $objs) == 2*($ObjSize+25)  &&
     get_bytesd('SharedStringClass', $objs) == 2*($ObjSize+16)+(2*(9/3)) ?
      "(GOOD) Bytes (SharedString) works\n" :
      "(BAD) Bytes (SharedString) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass2 = null;


// TEST: multiple ref counted arrays
$my_arr = array(
  getStr(4) => getStr(8), // 20:20 + 24:24 = 44:44
  getStr(5) => getStr(7), // 21:21 + 23:23 = 44:44
);
$myClass = new SharedArrayClass($my_arr);
$myClass2 = new SharedArrayClass($my_arr);
$my_arr = null;
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SharedArrayClass', $objs) ==
      2 * ($ObjSize + 88 + 16 /*(tv)*/ + 16 /*(ArrayData)*/)  &&
     get_bytesd('SharedArrayClass', $objs) ==
      2 * ($ObjSize + (88 + 16 /*(ArrayData)*/)/2 + 16 /*(tv)*/) ?
        "(GOOD) Bytes (SharedArray) works\n" :
        "(BAD) Bytes (SharedArray) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass2 = null;

// TEST: multiple ref counted nested arrays
$my_arr = array( // 16 /*(tv)*/ + 16 /*(ArrayData)*/ + 76 = 108
  array(getStr(4) => getStr(8)), // 20 + 24 + 16 /*(tv)*/ + 16 /*(ArrayData)*/
);
$myClass = new NestedArrayClass($my_arr);
$my_arr = null;
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes_eq('NestedArrayClass', $objs) == ($ObjSize + 108) ?
  "(GOOD) Bytes (NestedArray) works\n" :
  "(BAD) Bytes (NestedArray) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass = new SimpleMapClass();
$myClass2 = new SimpleMapClass();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SimpleMapClass', $objs) ==
      2 * ($ObjSize + 16 /*(tv)*/ + $MapSize + 39 + 43) &&
     get_bytesd('SimpleMapClass', $objs) ==
      2 * ($ObjSize + 16 /*(tv)*/ + $MapSize + 36 + 40) ?
      "(GOOD) Bytes (SimpleMapClass) works\n" :
      "(BAD) Bytes (SimpleMapClass) failed: " . var_export($objs, true) . "\n";
$objs = null;
$myClass = null;
$myClass2 = null;


// TEST: multiple ref counted Maps
$shared_map = Map{ // size = 16(tv)+$MapSize+39+43 = 122
                   // sized = 16(tv)+($MapSize+36+40)/2 = 66
  'foo' => getStr(4), // size = 19+20 = 39
                      // sized = 16+20 = 36
  'bar' => getStr(8), // size = 19+24 = 43
                      // sized = 16+24 = 40
};
$my_obj1 = new SharedMapClass($shared_map);
$my_obj2 = new SharedMapClass($shared_map);
$shared_map = null;
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SharedMapClass', $objs) ==
      2 * ($ObjSize + 16 /*(tv)*/ + $MapSize + 39 + 43) &&
     get_bytesd('SharedMapClass', $objs) ==
      2 * ($ObjSize + 16 /*(tv)*/ + ($MapSize + 36 + 40) / 2) ?
      "(GOOD) Bytes (SharedMapClass) works\n" :
      "(BAD) Bytes (SharedMapClass) failed: " . var_export($objs, true) . "\n";
$objs = null;
$my_obj1 = null;
$my_obj2 = null;
$my_obj = new SimpleMapClassWithBackEdge();
$my_obj->map['foo'] = $my_obj; // size = 19+16(tv) = 35, sized = 16+16(tv) = 32
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_bytes('SimpleMapClassWithBackEdge', $objs) ==
      ($ObjSize + 16 /*(tv)*/ + $MapSize + 39 + 35) &&
     get_bytesd('SimpleMapClassWithBackEdge', $objs) ==
      ($ObjSize + 16 /*(tv)*/ + $MapSize + 36 + 32) ?
        "(GOOD) Bytes (SimpleMapClassWithBackEdge) works\n" :
        "(BAD) Bytes (SimpleMapClassWithBackEdge) failed: " .
        var_export($objs, true) . "\n";
$objs = null;
$my_obj = null;

$my_obj = new SimpleClassForExclude();
$objs = objprof_get_data(OBJPROF_FLAGS_DEFAULT);
$exclude_class_instances_before = get_instances('ExlcudeClass', $objs);
$exclude_class_bytes_before = get_bytes('ExlcudeClass', $objs);
$exclude_class_bytesd_before = get_bytesd('ExlcudeClass', $objs);
$parent_class_bytes_before = get_bytes('SimpleClassForExclude', $objs);
$parent_class_bytesd_before =get_bytesd('SimpleClassForExclude', $objs);

$my_obj = new SimpleClassForExclude();
$objs = objprof_get_data(OBJPROF_FLAGS_DEFAULT, array('ExlcudeClass'));
$exclude_class_instances_after = get_instances('ExlcudeClass', $objs);
$parent_class_bytes_after = get_bytes('SimpleClassForExclude', $objs);
$parent_class_bytesd_after = get_bytesd('SimpleClassForExclude', $objs);

echo  $exclude_class_instances_before == 2 &&
      $exclude_class_instances_after == 0 &&
      $parent_class_bytes_after == ($parent_class_bytes_before +
        $exclude_class_bytes_before) &&
      $parent_class_bytesd_after == ($parent_class_bytesd_before +
        $exclude_class_bytesd_before) ?
    "(GOOD) Bytes (SimpleClassForExclude) works in DEFAULT mode\n"
  : "(BAD) Bytes (SimpleClassForExclude) failed: ".
    var_export($objs, true) . "\n";
$objs = null;
$my_obj = null;

$my_obj = new SimpleClassForExclude();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
$exclude_class_instances_before = get_instances('ExlcudeClass', $objs);
$exclude_class_bytes_before = get_bytes('ExlcudeClass', $objs);
$exclude_class_bytesd_before = get_bytesd('ExlcudeClass', $objs);
$parent_class_bytes_before = get_bytes('SimpleClassForExclude', $objs);
$parent_class_bytesd_before = get_bytesd('SimpleClassForExclude', $objs);

$my_obj = new SimpleClassForExclude();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY, array('ExlcudeClass'));
$exclude_class_instances_after = get_instances('ExlcudeClass', $objs);
$parent_class_bytes_after = get_bytes('SimpleClassForExclude', $objs);
$parent_class_bytesd_after = get_bytesd('SimpleClassForExclude', $objs);

echo  $exclude_class_instances_before == 2 &&
      $exclude_class_instances_after == 0 &&
      $parent_class_bytes_after == ($parent_class_bytes_before +
        $exclude_class_bytes_before) &&
      $parent_class_bytesd_after == ($parent_class_bytesd_before +
        $exclude_class_bytesd_before) ?
    "(GOOD) Bytes (SimpleClassForExclude) works in USER_TYPES_ONLY mode\n"
  : "(BAD) Bytes (SimpleClassForExclude) failed: ".
    var_export($objs, true) . "\n";
$objs = null;
$my_obj = null;


// LAST TEST: Dont crash on custom types
//$xml = simplexml_load_string('<root><hello>world</hello></root>');
//$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
echo "(GOOD) Got here without crashing\n";
}
