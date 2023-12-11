<?hh

class A {
  public $a = 5;
  public $b;
  function __construct($b = 'foo') { $this->b = $b; }
}

class B {
  public $a = 5;
  public $b;
  function __construct($b = 'foo') { $this->b = $b; }
}

class C {
  public $a = 5;
}

class DateTime1 implements DateTimeInterface {
  public $timestamp = 0;
  function __construct($timestamp) { $this->timestamp = $timestamp; }
  function getTimestamp() :mixed{
    if ($this->timestamp <= 0) {
      throw new Exception('sneaky');
    }
    return $this->timestamp;
  }
  function diff($dt, $absolute = null) :mixed{}
  function format($format) :mixed{}
  function getTimezone() :mixed{}
  function getOffset() :mixed{}
}

class DateTime2 implements DateTimeInterface {
  function getTimestamp() :mixed{ return 100; }
  function diff($dt, $absolute = null) :mixed{}
  function format($format) :mixed{}
  function getTimezone() :mixed{}
  function getOffset() :mixed{}
}

function foo(): void {}
function rfoo<reify T>(): void {}

class Bar {
  public static function foo(): void {}
  public static function rfoo<reify T>(): void {}
}

function test_pair($k1, $v1, $k2, $v2) :mixed{
  echo "$k1 cmp $k2:\n";
  try {
    echo (($v1 === $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 !== $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 == $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 != $v2) ? "T" : "F");
  } catch (Exception $e) {
    echo "Err";
  }

  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  error_reporting(0);

  $a1 = new A;
  $a2 = new A;
  $a3 = new A(new A);
  $a4 = new A(new A);
  $a5 = new A('bar');
  $a9 = new A(99);
  $a11 = new A(new DateTime1(1000));
  $a12 = new A(new DateTime1(1000));
  $a13= new A(new DateTime1(100));
  $a14 = new A(new DateTime1(-1));
  $a15 = new A(new DateTime2);
  $a16 = new A; $a16->c = 999;
  $a17 = new A(NAN);
  $a18 = new A(NAN);

  $b1 = new B;
  $c1 = new C;

  $t1 = new DateTime1(1000);
  $t2 = new DateTime1(1000);
  $t3 = new DateTime1(100);
  $t4 = new DateTime1(-1);
  $t5 = new DateTime2;

  $xml = simplexml_load_string("<root />")->unknown;

  $v1 = Vector{0, 1, 2, 3, 4};
  $v2 = Vector{0, 1, 2, 3, 4};
  $v3 = Vector{5, 6, 7, 8, 9};

  $p1 = Pair{'elem1', 'elem2'};
  $p2 = Pair{'elem1', 'elem2'};
  $p3 = Pair{'other1', 'other2'};

  $clo1 = function () { return 0; };
  $clo2 = function () { return 0; };

  $fp = foo<>;
  $rfp = rfoo<int>;
  $clsmeth = Bar::foo<>;
  $rclsmeth = Bar::rfoo<int>;

  $arr1 = vec[];
  $arr2 = vec[99];
  $arr3 = vec['foo'];
  $arr4 = vec['foo', 'bar'];
  $arr5 = vec['foo', 'bar'];
  $arr6 = vec['foo', 'baz'];
  $arr7 = vec[new A, new A];
  $arr8 = vec[new A, new A];
  $arr9 = vec[new A, new C];
  $arr12 = vec[vec[1, 2], vec[1, 2, 3]];
  $arr13 = vec[vec[1, 2], vec[1, 2, 3]];
  $arr14 = vec[vec[1, 2], vec[99]];
  $arr15 = vec[Vector{0, 1, 2, 3, 4}, Vector{5, 6, 7, 8}];
  $arr16 = vec[Vector{0, 1, 2, 3, 4}, Vector{5, 6, 7, 8}];
  $arr17 = dict['key1' => 1, 'key2' => 2, 'key3' => 3];
  $arr18 = dict['key1' => 1, 'key2' => 2, 'key3' => 3];
  $arr19 = dict['key1' => 1, 'key2-other' => 2, 'key3' => 3];
  $arr20 = dict['key2' => 2, 'key3' => 3, 'key1' => 1];
  $arr21 = vec['baz', 'foo'];

  $vec1 = vec[];
  $vec2 = vec[1, 2];
  $vec3 = vec[1, 2, 3];
  $vec4 = vec[1, 2, 4];
  $vec7 = vec[new A];
  $vec8 = vec[new A];
  $vec9 = vec[4, 2, 1];
  $vec10 = vec[1, 2, 'value'];

  $dict1 = dict[];
  $dict2 = dict['a' => 0, 'b' => 1];
  $dict3 = dict['a' => 0, 'b' => 1, 'c' => 2];
  $dict4 = dict['a' => 0, 'b' => 1, 'c' => 3];
  $dict5 = dict['a' => 0, 'z' => 1, 'c' => 3];
  $dict8 = dict[0 => new A];
  $dict9 = dict[0 => new A];
  $dict10 = dict['c' => 2, 'b' => 1, 'a' => 0];

  $ks1 = keyset[];
  $ks2 = keyset[1];
  $ks3 = keyset[1];
  $ks4 = keyset['a'];
  $ks5 = keyset['a'];
  $ks6 = keyset[1, 2, 3];
  $ks7 = keyset[3, 2, 1];
  $ks8 = keyset['a', 'b', 'c'];
  $ks9 = keyset['c', 'b', 'a'];
  $ks10 = keyset[1, 2, 4];
  $ks11 = keyset['a', 'b', 'd'];
  $ks12 = keyset[100, 2, 3];
  $ks13 = keyset['z', 'b', 'c'];

  $f1 = imagecreate(10, 10);
  $f2 = imagecreate(10, 10);
  $f3 = imagecreate(1, 1);

  $arr = dict['null' => null,

               'false' => false, 'true' => true,

               'int 0' => 0, 'int 99' => 99, 'int -1' => -1,

               'float 0' => 0.0,'double 99' => (float)99,
               'float 99' => 99.0, 'float -1' => -1.0,
               'INF' => INF, '-INF' => -INF, 'NAN' => NAN,

               '""' => "", '"0"' => "0",
               '"99"' => "99", '"-1"' => "-1", '"0.0"' => "0.0",
               '"99.0"' => "99.0", '"-1.0"' => "-1.0",
               '"foo"' => "foo", '"BAZ"' => "BAZ",

               'array arr1' => $arr1, 'array arr2' => $arr2,
               'array arr3' => $arr3, 'array arr4' => $arr4,
               'array arr5' => $arr5, 'array arr6' => $arr6,
               'array arr7' => $arr7, 'array arr8' => $arr8,
               'array arr9' => $arr9, 'array arr12' => $arr12,
               'array arr13' => $arr13, 'array arr14' => $arr14,
               'array arr15' => $arr15, 'array arr16' => $arr16,
               'array arr17' => $arr17, 'array arr18' => $arr18,
               'array arr19' => $arr19, 'array arr20' => $arr20,
               'array arr21' => $arr21,

               'vec vec1' => $vec1, 'vec vec2' => $vec2, 'vec vec3' => $vec3,
               'vec vec4' => $vec4, 'vec vec7' => $vec7, 'vec vec8' => $vec8,
               'vec vec9' => $vec9, 'vec vec10' => $vec10,

               'dict dict1' => $dict1, 'dict dict2' => $dict2, 'dict dict3' => $dict3,
               'dict dict4' => $dict4, 'dict dict5' => $dict5, 'dict dict8' => $dict8,
               'dict dict9' => $dict9, 'dict dict10' => $dict10,

               'keyset ks1' => $ks1, 'keyset ks2' => $ks2, 'keyset ks3' => $ks3,
               'keyset ks4' => $ks4, 'keyset ks5' => $ks5, 'keyset ks6' => $ks6,
               'keyset ks7' => $ks7, 'keyset ks8' => $ks8, 'keyset ks9' => $ks9,
               'keyset ks10' => $ks10, 'keyset ks11' => $ks11, 'keyset ks12' => $ks12,
               'keyset ks13' => $ks13,

               'object a1' => $a1, 'object a2' => $a2, 'object a3' => $a3,
               'object a4' => $a4, 'object a5' => $a5, 'object a9' => $a9,
               'object a11' => $a11, 'object a12' => $a12,
               'object a13' => $a13, 'object a14' => $a14, 'object a15' => $a15,
               'object a16' => $a16, 'object a17' => $a17, 'object a18' => $a18,
               'object b1' => $b1, 'object c1' => $c1,
               'object t1' => $t1, 'object t2' => $t2, 'object t3' => $t3,
               'object t4' => $t4, 'object t5' => $t5, 'object xml' => $xml,

               'vector v1' => $v1, 'vector v2' => $v2, 'vector v3' => $v3,
               'pair p1' => $p1, 'pair p2' => $p2, 'pair p3' => $p3,

               'closure clo1' => $clo1, 'closure clo2' => $clo2,

               'resource f1' => $f1, 'resource f2' => $f2, 'resource f3' => $f3,

               'func_ptr' => $fp, 'rfunc_ptr' => $rfp,
               'clsmeth' => $clsmeth, 'rclsmeth' => $rclsmeth,
              ];

  echo "same    nsame   eq      neq\n\n";
  foreach ($arr as $k1 => $v1) {
    foreach ($arr as $k2 => $v2) {
      test_pair($k1, $v1, $k2, $v2);
    }
  }

  imagedestroy($f1);
  imagedestroy($f2);
  imagedestroy($f3);
}
