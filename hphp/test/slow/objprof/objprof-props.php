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
class SharedStringClass {
  public string $val_ref = null;
  public function __construct(string $str) {
    $this->val_ref = $str; // inc 2 + inc 3
  }
}


class ExlcudeClass {}
class SimpleClassForExclude {
  public Map<string,mixed> $map;
  public ExlcudeClass $fooCls1;
  public ExlcudeClass $fooCls2;
  public function __construct() {
    $this->map = Map{ // $MapSize + 39:36 + 43:40
      'foo' => getStr(4), // 19:16 + 20:20 = 39:36
      'bar' => getStr(8), // 19:16 + 24:24 = 43:40
    };
    $this->fooCls1 = new ExlcudeClass(); // $ObjSize
    $this->fooCls2 = new ExlcudeClass(); // $ObjSize
  }
}

<<__EntryPoint>>
function main_objprof_props() {
$myClass = new EmptyClass();
$objs = objprof_get_data(OBJPROF_FLAGS_USER_TYPES_ONLY);
$emptyCount = get_instances("EmptyClass", $objs);
$ObjSize = get_bytes("EmptyClass", $objs) / $emptyCount;
$myClass = null;
$objs = null;
$myClass = new SimpleProps();
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('SimpleProps::prop1', $objs) == 1 &&
     get_instances('SimpleProps::prop2', $objs) == 1 &&
     get_instances('SimpleProps::prop3', $objs) == 1 &&
     get_bytes('SimpleProps::prop1', $objs) == 19 &&
     get_bytes('SimpleProps::prop2', $objs) == 16 &&
     get_bytes('SimpleProps::prop2', $objs) == 16 &&
     get_bytesd('SimpleProps::prop1', $objs) == 16 &&
     get_bytesd('SimpleProps::prop2', $objs) == 16 &&
     get_bytesd('SimpleProps::prop2', $objs) == 16 ?
      "(GOOD) Bytes (props) works\n" :
      "(BAD) Bytes (props) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;
$myClass = new SimpleArrays();
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('SimpleArrays::arrEmpty', $objs) == 1 &&
     get_instances('SimpleArrays::arrMixed', $objs) == 1 &&
     get_instances('SimpleArrays::arrNums', $objs) == 1 &&
     get_bytes('SimpleArrays::arrEmpty', $objs) == 32 &&
     get_bytes('SimpleArrays::arrMixed', $objs) == 110 &&
     get_bytes('SimpleArrays::arrNums', $objs) == 80 &&
     get_bytesd('SimpleArrays::arrEmpty', $objs) == 16 &&
     get_bytesd('SimpleArrays::arrMixed', $objs) == 16 &&
     get_bytesd('SimpleArrays::arrNums', $objs) == 16 ?
      "(GOOD) Bytes (arrays) works\n" :
      "(BAD) Bytes (arrays) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;
$myClass = new DynamicClass();
$dynamic_field = 'abcd'; // 20:16
$dynamic_field2 = 1234;  // 20:16 (dynamic properties - always string)
$myClass->$dynamic_field = 1; // 16:16
$myClass->$dynamic_field2 = 1; // 16:16
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('DynamicClass::abcd', $objs) == 1 &&
     get_instances('DynamicClass::1234', $objs) == 1 &&
     get_bytes('DynamicClass::abcd', $objs) == 36 &&
     get_bytes('DynamicClass::1234', $objs) == 36 &&
     get_bytesd('DynamicClass::abcd', $objs) == 32 &&
     get_bytesd('DynamicClass::1234', $objs) == 32 ?
      "(GOOD) Bytes (dynamic) works\n" :
      "(BAD) Bytes (dynamic) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;

$myClass = Map{};
$MapSize = get_bytes('HH\Map', objprof_get_data(OBJPROF_FLAGS_DEFAULT));

// TEST: map with int and string keys (Mixed). DEFAULT mode.
$myClass = Map {
  "abc" => 1, // 35:32
  1 => "22", // 34:32
  1234123 => 3 // 32:32
};
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('HH\Map::abc', $objs) == 1 &&
     get_instances('HH\Map::1', $objs) == 1 &&
     get_instances('HH\Map::1234123', $objs) == 1 &&
     get_bytes('HH\Map::abc', $objs) == 35 &&
     get_bytes('HH\Map::1', $objs) == 34 &&
     get_bytes('HH\Map::1234123', $objs) == 32 &&
     get_bytesd('HH\Map::abc', $objs) == 32 &&
     get_bytesd('HH\Map::1', $objs) == 32 &&
     get_bytesd('HH\Map::1234123', $objs) == 32 ?
      "(GOOD) Bytes (Mixed Map) works in default mode\n" :
      "(BAD) Bytes (Mixed Map) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;


// TEST: map with int and string keys (Mixed). USER_TYPES_ONLY mode.
$myClass = Map {
  "abc" => 1, // 3 + 16 + 16 = 35
  1 => "22", // 16 + 16 + 2 = 34
  1234123 => 3 // 16 + 16 = 32
};
$objs =
  objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY | OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_instances('HH\Map::abc', $objs) == 0 &&
     get_instances('HH\Map::1', $objs) == 0 &&
     get_instances('HH\Map::1234123', $objs) == 0 ?
      "(GOOD) Bytes (Mixed Map) works in USER_TYPES_ONLY mode\n" :
      "(BAD) Bytes (Mixed Map) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;


// TEST: vector with int and string vals (Packed). DEFAULT mode.
$myClass = Vector {
  "abc", // 19:16
  1, // 16:16
};
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('HH\Vector::<index>', $objs) == 2 &&
     get_bytes('HH\Vector::<index>', $objs) == 35 &&
     get_bytesd('HH\Vector::<index>', $objs) == 32 ?
      "(GOOD) Bytes (Vector) works in default mode\n" :
      "(BAD) Bytes (Vector) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;


// TEST: vector with int and string vals (Packed). USER_TYPES_ONLY mode.
$myClass = Vector {
  "abc", // 19:16
  1, // 16:16
};
$objs =
  objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY | OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_instances('HH\Vector::0', $objs) == 0 &&
     get_instances('HH\Vector::1', $objs) == 0 ?
      "(GOOD) Bytes (Vector) works in USER_TYPES_ONLY mode\n" :
      "(BAD) Bytes (Vector) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;


// TEST: multiple ref counted strings
$mystr = getStr(9); // inc 1, 25:16
$myClass = new SharedStringClass($mystr);
$myClass2 = new SharedStringClass($mystr);
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('SharedStringClass::val_ref', $objs) == 2 &&
     get_bytes('SharedStringClass::val_ref', $objs) == (25*2) &&
     get_bytesd('SharedStringClass::val_ref', $objs) == (16*2 + (9/3)*2) ?
      "(GOOD) Bytes (SharedString) works\n" :
      "(BAD) Bytes (SharedString) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$myClass2 = null;
$objs = null;
$myClass = new SimpleClassForExclude();
$objs = objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY);
echo get_instances('SimpleClassForExclude::map', $objs) == 1 &&
     get_instances('SimpleClassForExclude::fooCls1', $objs) == 1 &&
     get_instances('SimpleClassForExclude::fooCls2', $objs) == 1 &&
     get_instances('HH\Map::foo', $objs) == 1 &&
     get_instances('HH\Map::bar', $objs) == 1 &&
     get_bytes('SimpleClassForExclude::map', $objs) == 16 /* (tv) */ &&
     get_bytes('SimpleClassForExclude::fooCls1', $objs) == 16 /* (tv) */ &&
     get_bytes('SimpleClassForExclude::fooCls2', $objs) == 16 /* (tv) */ &&
     get_bytes('HH\Map::foo', $objs) == 39 &&
     get_bytes('HH\Map::bar', $objs) == 43 &&
     get_bytesd('SimpleClassForExclude::map', $objs) == 16 &&
     get_bytesd('SimpleClassForExclude::fooCls1', $objs) == 16 &&
     get_bytesd('SimpleClassForExclude::fooCls2', $objs) == 16 &&
     get_bytesd('HH\Map::foo', $objs) == 36 &&
     get_bytesd('HH\Map::bar', $objs) == 40 ?
      "(GOOD) Bytes (Mixed Class Props) works in default mode\n" :
      "(BAD) Bytes (Mixed Map) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;

$myClass = new SimpleClassForExclude();
$objs =
  objprof_get_data(OBJPROF_FLAGS_PER_PROPERTY | OBJPROF_FLAGS_USER_TYPES_ONLY);
echo get_instances('SimpleClassForExclude::map', $objs) == 1 &&
     get_instances('SimpleClassForExclude::fooCls1', $objs) == 1 &&
     get_instances('SimpleClassForExclude::fooCls2', $objs) == 1 &&
     get_bytes('SimpleClassForExclude::map', $objs) == (16+$MapSize+39+43) &&
     get_bytes('SimpleClassForExclude::fooCls1', $objs) == 16 /* (tv) */  &&
     get_bytes('SimpleClassForExclude::fooCls2', $objs) == 16 /* (tv) */  &&
     get_bytesd('SimpleClassForExclude::map', $objs) == (16+$MapSize+36+40) &&
     get_bytesd('SimpleClassForExclude::fooCls1', $objs) == 16 &&
     get_bytesd('SimpleClassForExclude::fooCls2', $objs) == 16 ?
      "(GOOD) Bytes (Mixed Class Props) works in USER_TYPES_ONLY mode\n" :
      "(BAD) Bytes (Mixed Map) failed: " . var_export($objs, true) . "\n";
$myClass = null;
$objs = null;
}
