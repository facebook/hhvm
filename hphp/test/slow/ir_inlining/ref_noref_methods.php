<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 {

  private static $get_res = dict['type' => 1];
  <<__NEVER_INLINE>>
  public function get() :mixed{
    return self::$get_res;
  }
}

class Cls2 {

  private static $get_res = dict['type' => 2];
  <<__NEVER_INLINE>>
  public function get() :mixed{
    return self::$get_res;
  }
}

function run($x) :mixed{
  $a = $x->get();
  return $a['type'];
}

function inline_run() :mixed{
  run(new Cls2());
}


// Profile a region within run() for both cases where $x->get() returns a ref
// and not.
<<__EntryPoint>>
function main_ref_noref_methods() :mixed{
$b = new Cls1();
$d = new Cls2();
for ($i = 0; $i < 100; $i++) {
  run($b);
  run($d);
}

// Now inline run() with an argument that will always fail one of the guards in
// the region. We shouldn't choke on incompatible types in the unreachable
// region.
for ($i = 0; $i < 200; $i++) {
  inline_run();
}

echo "DONE\n";
}
