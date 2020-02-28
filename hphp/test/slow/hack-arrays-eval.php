<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_eval() {
  echo "============ test_eval =======================================\n";

  eval('$v = vec[1, 2, 3];');
  eval('$d = dict[100 => \'abc\', 200 => \'def\'];');
  eval('$k = keyset[1, \'abc\', 3];');

  var_dump($v);
  var_dump($d);
  var_dump($k);

  $a = varray[100, 200, 300];
  eval('$v = vec($a);');
  eval('$d = dict($a);');
  eval('$k = keyset($a);');

  var_dump($v);
  var_dump($d);
  var_dump($k);

  eval('var_dump(is_vec($v));');
  eval('var_dump(is_dict($d));');
  eval('var_dump(is_keyset($k));');
}
function wrap($arr, $v) { return $arr[] = $v; }
function test_func($v = wrap(varray[], vec[1, 2]),
                   $d = wrap(varray[], dict[100 => 'a', 200 => 'b']),
                   $k = wrap(varray[], keyset['a', 'b'])) {}

function test_default_value() {
  echo "============ test_default_value ==============================\n";
  $r = new ReflectionFunction('test_func');
  var_dump($r->getParameters()[0]->info['default']);
  var_dump($r->getParameters()[1]->info['default']);
  var_dump($r->getParameters()[2]->info['default']);
}


<<__EntryPoint>>
function main_hack_arrays_eval() {
test_eval();
test_default_value();
}
