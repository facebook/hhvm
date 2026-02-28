<?hh

class Cls1 implements IMemoizeParam {
  public function __construct(public string $x)[] {}
  public function getInstanceKey(): string {
    echo "Cls1::getInstanceKey\n"; return $this->x;
  }
}class Cls2 implements IMemoizeParam {
  public function __construct(public int $x)[] {}
  public function getInstanceKey(): int {
    echo "Cls2::getInstanceKey\n"; return $this->x;
  }
}class Cls3 implements IMemoizeParam {
  public function __construct(public $x)[] {}
  public function getInstanceKey() :mixed{
    echo "Cls3::getInstanceKey\n"; return $this->x;
  }
}class Cls4 {}
<<__Memoize>> function takes_int(int $x)          :mixed{ echo "takes_int\n"; return $x; }
<<__Memoize>> function takes_bool(bool $x)        :mixed{ echo "takes_bool\n"; return $x; }
<<__Memoize>> function takes_str(string $x)       :mixed{ echo "takes_str\n"; return $x; }
<<__Memoize>> function takes_double(float $x)    :mixed{ echo "takes_double\n"; return $x; }
<<__Memoize>> function takes_arrkey(arraykey $x)  :mixed{ echo "takes_arrkey\n"; return $x; }
<<__Memoize>> function takes_vector(Vector $x)    :mixed{ echo "takes_vector\n"; return $x; }
<<__Memoize>> function takes_cls1(Cls1 $x)        :mixed{ echo "takes_cls1\n"; return $x; }
<<__Memoize>> function takes_cls2(Cls2 $x)        :mixed{ echo "takes_cls2\n"; return $x; }
<<__Memoize>> function takes_cls3(Cls3 $x)        :mixed{ echo "takes_cls3\n"; return $x; }
<<__Memoize>> function takes_cls4(Cls4 $x)        :mixed{ echo "takes_cls4\n"; return $x; }
<<__Memoize>> function takes_opt_int(?int $x)     :mixed{ echo "takes_opt_int\n"; return $x; }
<<__Memoize>> function takes_opt_bool(?bool $x)   :mixed{ echo "takes_opt_bool\n"; return $x; }
<<__Memoize>> function takes_opt_str(?string $x)  :mixed{ echo "takes_opt_str\n"; return $x; }
<<__Memoize>> function takes_opt_dbl(?float $x)  :mixed{ echo "takes_opt_dbl\n"; return $x; }
<<__Memoize>> function takes_opt_vector(?Vector $x) :mixed{ echo "takes_opt_vector\n"; return $x; }
<<__Memoize>> function takes_opt_cls1(?Cls1 $x)   :mixed{ echo "takes_opt_cls1\n"; return $x; }
<<__Memoize>> function takes_opt_cls2(?Cls2 $x)   :mixed{ echo "takes_opt_cls2\n"; return $x; }
<<__Memoize>> function takes_opt_cls3(?Cls3 $x)   :mixed{ echo "takes_opt_cls3\n"; return $x; }
<<__Memoize>> function takes_opt_cls4(?Cls4 $x)   :mixed{ echo "takes_opt_cls4\n"; return $x; }
<<__Memoize>> function takes_anything($x)         :mixed{ echo "takes_anything\n"; return $x; }

function test() :mixed{
  var_dump(takes_int(__hhvm_intrinsics\launder_value(123)));
  var_dump(takes_bool(__hhvm_intrinsics\launder_value(true)));
  var_dump(takes_str(__hhvm_intrinsics\launder_value('abc')));
  var_dump(takes_double(__hhvm_intrinsics\launder_value(3.141)));
  var_dump(takes_arrkey(__hhvm_intrinsics\launder_value(707)));
  var_dump(takes_vector(__hhvm_intrinsics\launder_value(Vector{1, 2, 3})));
  var_dump(takes_cls1(__hhvm_intrinsics\launder_value(new Cls1('xyz'))));
  var_dump(takes_cls2(__hhvm_intrinsics\launder_value(new Cls2(311))));
  var_dump(takes_cls3(__hhvm_intrinsics\launder_value(new Cls3(true))));
  var_dump(takes_cls3(__hhvm_intrinsics\launder_value(new Cls3(new Cls1('aaaa')))));

  try {
    var_dump(takes_cls4(__hhvm_intrinsics\launder_value(new Cls4())));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  var_dump(takes_opt_int(__hhvm_intrinsics\launder_value(123)));
  var_dump(takes_opt_bool(__hhvm_intrinsics\launder_value(true)));
  var_dump(takes_opt_str(__hhvm_intrinsics\launder_value('abc')));
  var_dump(takes_opt_dbl(__hhvm_intrinsics\launder_value(3.141)));
  var_dump(takes_opt_vector(__hhvm_intrinsics\launder_value(Vector{1, 2, 3})));
  var_dump(takes_opt_cls1(__hhvm_intrinsics\launder_value(new Cls1('xyz'))));
  var_dump(takes_opt_cls2(__hhvm_intrinsics\launder_value(new Cls2(311))));
  var_dump(takes_opt_cls3(__hhvm_intrinsics\launder_value(new Cls3(true))));
  var_dump(takes_opt_cls3(__hhvm_intrinsics\launder_value(new Cls3(new Cls1('aaaa')))));

  try {
    var_dump(takes_opt_cls4(__hhvm_intrinsics\launder_value(new Cls4())));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  var_dump(takes_int(__hhvm_intrinsics\launder_value(987)));
  var_dump(takes_bool(__hhvm_intrinsics\launder_value(false)));
  var_dump(takes_str(__hhvm_intrinsics\launder_value('foo')));
  var_dump(takes_double(__hhvm_intrinsics\launder_value(9.871)));
  var_dump(takes_arrkey(__hhvm_intrinsics\launder_value('bar')));
  var_dump(takes_vector(__hhvm_intrinsics\launder_value(Vector{7, 8, 9})));
  var_dump(takes_cls1(__hhvm_intrinsics\launder_value(new Cls1('biz'))));
  var_dump(takes_cls2(__hhvm_intrinsics\launder_value(new Cls2(1001))));
  var_dump(takes_cls3(__hhvm_intrinsics\launder_value(new Cls3(false))));
  var_dump(takes_cls3(__hhvm_intrinsics\launder_value(new Cls3(new Cls2(12321)))));

  try {
    var_dump(takes_cls4(__hhvm_intrinsics\launder_value(new Cls4())));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  var_dump(takes_opt_int(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_bool(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_str(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_dbl(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_vector(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_cls1(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_cls2(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_cls3(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_opt_cls4(__hhvm_intrinsics\launder_value(null)));

  var_dump(takes_anything(__hhvm_intrinsics\launder_value(null)));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(true)));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(false)));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(500)));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value('a-string')));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value('')));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(vec[1, 2, 3])));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(vec[4, 5, 6])));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(dict['a' => 1, 'b' => 2, 'c' => 3])));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(keyset['z', 'y', 'x'])));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(Vector{'abc', 'def'})));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls1('cls1-1'))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls1('cls1-2'))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls2(999))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls2(666))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls3(true))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls3(false))));
  var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls3(new Cls2(8732)))));

  try {
    var_dump(takes_anything(__hhvm_intrinsics\launder_value(new Cls4())));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_memo_key_specialization() :mixed{
set_error_handler(
  function($errno, $errstr) { echo "ERROR: " . $errstr . "\n"; }
);
;
;
;
;

test();
echo "=======================================\n";
test();
}
