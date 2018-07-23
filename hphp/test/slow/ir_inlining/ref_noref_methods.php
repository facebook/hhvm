<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 {
  <<__NEVER_INLINE>>
  public function &get() {
    static $_ = ['type' => 1];
    return $_;
  }
}

class Cls2 {
  <<__NEVER_INLINE>>
  public function get() {
    static $_ = ['type' => 2];
    return $_;
  }
}

function run($x) {
  $a = $x->get();
  return $a['type'];
}

function inline_run() {
  run(new Cls2());
}

// Profile a region within run() for both cases where $x->get() returns a ref
// and not.
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
