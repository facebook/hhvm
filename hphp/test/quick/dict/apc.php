<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class SomeClass {
  public $val;
}

class Sleep {
  public $val;

  function __construct($val) {
    $this->val = $val;
  }

  function __sleep() :mixed{
    echo "Sleep...\n";
    return vec[];
  }
}

class Wakeup {
  function __wakeup() :mixed{
    echo "Wakeup...\n";
    $this->val = 12345;
  }
}

class WakeupThrow {
  function __wakeup() :mixed{
    throw new Exception("Wakeup exception");
  }
}

function get_count() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check("count");
  if (!$count) {
    $count = 0;
  }
  $count++;
  apc_store("count", $count);
  return $count;
}

function read() :mixed{
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val1"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val2"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val3"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val4"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val5"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val6"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val7"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val8"));

  try {
    var_dump(__hhvm_intrinsics\apc_fetch_no_check("val9"));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val11"));
}

function write($count) :mixed{
  apc_store("val1", dict[]);
  apc_store("val2", dict["a" => 1, "b" => 2, "c" => 3]);
  apc_store("val3", dict[4 => "a", 5 => "b", 6 => "C", 7 => "D"]);
  apc_store("val4", dict[$count => 999, 999 => $count]);
  apc_store("val5", dict[0 => new stdClass, 1 => new stdClass]);

  $cls = new SomeClass;
  $cls->val = dict[50 => dict[], "50" => dict[100 => $count],
                   $count => dict[101 => $count, 102 => $count+1]];
  apc_store("val6", $cls);

  $cls2 = new SomeClass;
  $d = dict["cls" => $cls2];
  $cls2->val = $d;
  apc_store("val7", $cls2);

  apc_store("val8", dict[123 => new Wakeup]);
  apc_store("val9", dict["456" => new WakeupThrow]);
  apc_store("val11", dict['abc' => new Sleep(123)]);
}

<<__EntryPoint>> function main(): void {
  $count = get_count();
  echo "Count: $count\n";
  read();
  write($count);
  read();
}
