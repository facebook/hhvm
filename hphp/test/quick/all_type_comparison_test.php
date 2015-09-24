<?hh
error_reporting(0);

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

class ToString {
  public $str;
  function __construct($str = 'foo') { $this->str = $str; }
  function __toString() { return $this->str; }
}

class ToStringThrower {
  function __toString() { throw new Exception('sneaky'); }
}

class DateTime1 implements DateTimeInterface {
  public $timestamp = 0;
  function __construct($timestamp) { $this->timestamp = $timestamp; }
  function getTimestamp() {
    if ($this->timestamp <= 0) {
      throw new Exception('sneaky');
    }
    return $this->timestamp;
  }
  function diff($dt, $absolute = null) {}
  function format($format) {}
  function getTimezone() {}
  function getOffset() {}
}

class DateTime2 implements DateTimeInterface {
  function getTimestamp() { return 100; }
  function diff($dt, $absolute = null) {}
  function format($format) {}
  function getTimezone() {}
  function getOffset() {}
}

function test_pair($k1, $v1, $k2, $v2) {
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
    echo (($v1 < $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 <= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 == $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 != $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 >= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 > $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    $cmp = $v1 <=> $v2;
    echo "$cmp";
  } catch (Exception $e) {
    echo "Err";
  }

  echo "\n";
}

function test() {
  $a1 = new A;
  $a2 = new A;
  $a3 = new A(new A);
  $a4 = new A(new A);
  $a5 = new A('bar');
  $a6 = new A(new ToString);
  $a7 = new A(new ToString('bar'));
  $a8 = new A(new ToStringThrower);
  $a9 = new A(99);
  $a10 = new A(new ToString('99'));
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

  $s1 = new ToString;
  $s2 = new ToString('BAZ');
  $s3 = new ToString('99');
  $s4 = new ToStringThrower;

  $t1 = new DateTime1(1000);
  $t2 = new DateTime1(1000);
  $t3 = new DateTime1(100);
  $t4 = new DateTime1(-1);
  $t5 = new DateTime2;

  $ao1 = new ArrayObject(array());
  $ao2 = new ArrayObject(array(99));
  $ao3 = new ArrayObject(array(99));

  $xml = simplexml_load_string("<root />")->unknown;

  $v1 = Vector{0, 1, 2, 3, 4};
  $v2 = Vector{0, 1, 2, 3, 4};
  $v3 = Vector{5, 6, 7, 8, 9};

  $p1 = Pair{'elem1', 'elem2'};
  $p2 = Pair{'elem1', 'elem2'};
  $p3 = Pair{'other1', 'other2'};

  $clo1 = function () { return 0; };
  $clo2 = function () { return 0; };

  $arr1 = array();
  $arr2 = array(99);
  $arr3 = array('foo');
  $arr4 = array('foo', 'bar');
  $arr5 = array('foo', 'bar');
  $arr6 = array('foo', 'baz');
  $arr7 = array(new A, new A);
  $arr8 = array(new A, new A);
  $arr9 = array(new A, new C);
  $arr10 = array(new ToString('foo'), new ToString('bar'));
  $arr11 = array(new ToString('foo'), new ToStringThrower);
  $arr12 = array(array(1, 2), array(1, 2, 3));
  $arr13 = array(array(1, 2), array(1, 2, 3));
  $arr14 = array(array(1, 2), array(99));
  $arr15 = array(Vector{0, 1, 2, 3, 4}, Vector{5, 6, 7, 8});
  $arr16 = array(Vector{0, 1, 2, 3, 4}, Vector{5, 6, 7, 8});
  $arr17 = array(1, NAN);
  $arr18 = array(1, NAN);
  $arr19 = array(NAN, 1);
  $arr20 = array(1, NAN, 2);
  $arr21 = array('key1' => 1, 'key2' => 2, 'key3' => 3);
  $arr22 = array('key1' => 1, 'key2' => 2, 'key3' => 3);
  $arr23 = array('key1' => 1, 'key2-other' => 2, 'key3' => 3);

  $f1 = imagecreate(10, 10);
  $f2 = imagecreate(10, 10);
  $f3 = imagecreate(1, 1);

  $arr = array('null' => null,

               'false' => false, 'true' => true,

               'int 0' => 0, 'int 99' => 99, 'int -1' => -1,

               'float 0' => 0.0,'double 99' => (double)99,
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
               'array arr9' => $arr9, 'array arr10' => $arr10,
               'array arr11' => $arr11, 'array arr12' => $arr12,
               'array arr13' => $arr13, 'array arr14' => $arr14,
               'array arr15' => $arr15, 'array arr16' => $arr16,
               'array arr17' => $arr17, 'array arr18' => $arr18,
               'array arr19' => $arr19, 'array arr20' => $arr20,
               'array arr21' => $arr21, 'array arr22' => $arr22,
               'array arr23' => $arr23,

               'object a1' => $a1, 'object a2' => $a2, 'object a3' => $a3,
               'object a4' => $a4, 'object a5' => $a5, 'object a6' => $a6,
               'object a7' => $a7, 'object a8' => $a8, 'object a9' => $a9,
               'object a10' => $a10, 'object a11' => $a11, 'object a12' => $a12,
               'object a13' => $a13, 'object a14' => $a14, 'object a15' => $a15,
               'object a16' => $a16, 'object a17' => $a17, 'object a18' => $a18,
               'object b1' => $b1, 'object c1' => $c1, 'object s1' => $s1,
               'object s2' => $s2, 'object s3' => $s3, 'object s4' => $s4,
               'object t1' => $t1, 'object t2' => $t2, 'object t3' => $t3,
               'object t4' => $t4, 'object t5' => $t5, 'object ao1' => $ao1,
               'object ao2' => $ao2, 'object ao3' => $ao3, 'object xml' => $xml,

               'vector v1' => $v1, 'vector v2' => $v2, 'vector v3' => $v3,
               'pair p1' => $p1, 'pair p2' => $p2, 'pair p3' => $p3,

               'closure clo1' => $clo1, 'closure clo2' => $clo2,

               'resource f1' => $f1, 'resource f2' => $f2, 'resource f3' => $f3,
              );

  echo "same    nsame   lt      lte     eq      neq     gte     gt      cmp\n\n";
  foreach ($arr as $k1 => $v1) {
    foreach ($arr as $k2 => $v2) {
      test_pair($k1, $v1, $k2, $v2);
    }
  }

  imagedestroy($f1);
  imagedestroy($f2);
  imagedestroy($f3);
}

test();
