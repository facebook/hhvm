<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_eval() :mixed{
  echo "============ test_eval =======================================\n";

  eval('function f1() { return vec[1, 2, 3]; }');
  eval('function f2() { return dict[100 => \'abc\', 200 => \'def\']; }');
  eval('function f3() { return keyset[1, \'abc\', 3]; }');

  var_dump(f1());
  var_dump(f2());
  var_dump(f3());

  $a = vec[100, 200, 300];
  eval('function f4($a) { return vec($a); }');
  eval('function f5($a) { return dict($a); }');
  eval('function f6($a) { return keyset($a); }');

  $v = f4($a);
  $d = f5($a);
  $k = f6($a);

  var_dump($v);
  var_dump($d);
  var_dump($k);

  eval('function f7($v) { var_dump(is_vec($v)); }');
  eval('function f8($d) { var_dump(is_dict($d)); }');
  eval('function f9($k) { var_dump(is_keyset($k)); }');

  f7($v);
  f8($d);
  f9($k);
}
function wrap($arr, $v) :mixed{ return $arr[] = $v; }
function test_func($v = wrap(vec[], vec[1, 2]),
                   $d = wrap(vec[], dict[100 => 'a', 200 => 'b']),
                   $k = wrap(vec[], keyset['a', 'b'])) :mixed{}

function test_default_value() :mixed{
  echo "============ test_default_value ==============================\n";
  $r = new ReflectionFunction('test_func');
  var_dump($r->getParameters()[0]->info['default']);
  var_dump($r->getParameters()[1]->info['default']);
  var_dump($r->getParameters()[2]->info['default']);
}


<<__EntryPoint>>
function main_hack_arrays_eval() :mixed{
test_eval();
test_default_value();
}
