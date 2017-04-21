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

  $a = [100, 200, 300];
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

function test_assert() {
  echo "============ test_assert =====================================\n";

  assert('($v = vec[1, 2, 3]) || true');
  assert('($d = dict[100 => \'abc\', 200 => \'def\']) || true');
  assert('($k = keyset[1, \'abc\', 3]) || true');

  var_dump($v);
  var_dump($d);
  var_dump($k);

  $a = [100, 200, 300];
  assert('($v = vec($a)) || true');
  assert('($d = dict($a)) || true');
  assert('($k = keyset($a)) || true');

  var_dump($v);
  var_dump($d);
  var_dump($k);

  assert('(var_dump(is_vec($v))) || true');
  assert('(var_dump(is_dict($d))) || true');
  assert('(var_dump(is_keyset($k))) || true');
}

function test_create_function() {
  echo "============ test_create_function ============================\n";

  $f1 = create_function('', 'return vec[1, 2, 3];');
  $f2 = create_function('', 'return dict[100 => \'abc\', 200 => \'def\'];');
  $f3 = create_function('', 'return keyset[1, \'abc\', 3];');

  $v = $f1();
  $d = $f2();
  $k = $f3();

  var_dump($k);
  var_dump($d);
  var_dump($k);

  $a = [100, 200, 300];
  $f4 = create_function('$a', 'return vec($a);');
  $f5 = create_function('$a', 'return dict($a);');
  $f6 = create_function('$a', 'return keyset($a);');

  var_dump($f4($a));
  var_dump($f5($a));
  var_dump($f6($a));

  $f7 = create_function('$v', 'return is_vec($v);');
  $f8 = create_function('$d', 'return is_dict($d);');
  $f9 = create_function('$k', 'return is_keyset($k);');

  var_dump($f7($v));
  var_dump($f8($d));
  var_dump($f9($k));
}

$array = [];
function test_func($v = $array[] = vec[1, 2],
                   $d = $array[] = dict[100 => 'a', 200 => 'b'],
                   $k = $array[] = keyset['a', 'b']) {}

function test_default_value() {
  echo "============ test_default_value ==============================\n";
  $r = new ReflectionFunction('test_func');
  var_dump($r->getParameters()[0]->info['default']);
  var_dump($r->getParameters()[1]->info['default']);
  var_dump($r->getParameters()[2]->info['default']);
}

test_eval();
test_assert();
test_create_function();
test_default_value();
