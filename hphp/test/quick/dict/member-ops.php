<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function base_elem_warn($d, $k) :mixed{
  try {
    var_dump($d[$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function base_elem($d, $k) :mixed{
  try {
    var_dump($d[$k]['a'] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function base_define($d, $k) :mixed{
  try {
    $d[$k]['a'] = 200;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($d);
}

function base_unset($d, $k) :mixed{
  try {
    unset($d[$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($d);
}

function base($d) :mixed{
  echo "======= base_elem_warn =============================\n";
  base_elem_warn($d, 1);
  base_elem_warn($d, 3);
  base_elem_warn($d, '1');
  base_elem_warn($d, 'foo');
  base_elem_warn($d, true);

  echo "======= base_elem ==================================\n";
  base_elem($d, 1);
  base_elem($d, 3);
  base_elem($d, '1');
  base_elem($d, 'foo');
  base_elem($d, true);

  echo "======= base_define ================================\n";
  base_define($d, 1);
  base_define($d, 3);
  base_define($d, '1');
  base_define($d, 'foo');
  base_define($d, true);

  echo "======= base_unset =================================\n";
  base_unset($d, 1);
  base_unset($d, 3);
  base_unset($d, '1');
  base_unset($d, 'foo');
  base_unset($d, true);

  echo "======= base_new_elem_set ==========================\n";
  try {
    $copy = $d;
    $copy[]['a'] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= base_prop_get ==============================\n";
  try {
    var_dump($d->foobaz[0]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= base_prop_set ==============================\n";
  try {
    $copy = $d;
    $copy->foobaz = vec[123];
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_elem_warn($a, $k) :mixed{
  try {
    var_dump($a[1][$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_elem($a, $k) :mixed{
  try {
    var_dump($a[1][$k]['a'] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_define($a, $k) :mixed{
  try {
    $a[1][$k]['a'] = 200;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function dim_unset($a, $k) :mixed{
  try {
    unset($a[1][$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function dim($a) :mixed{
  echo "======= dim_elem_warn ==============================\n";
  dim_elem_warn($a, 1);
  dim_elem_warn($a, 3);
  dim_elem_warn($a, '1');
  dim_elem_warn($a, 'foo');
  dim_elem_warn($a, true);

  echo "======= dim_elem ===================================\n";
  dim_elem($a, 1);
  dim_elem($a, 3);
  dim_elem($a, '1');
  dim_elem($a, 'foo');
  dim_elem($a, true);

  echo "======= dim_define =================================\n";
  dim_define($a, 1);
  dim_define($a, 3);
  dim_define($a, '1');
  dim_define($a, 'foo');
  dim_define($a, true);

  echo "======= dim_unset ==================================\n";
  dim_unset($a, 1);
  dim_unset($a, 3);
  dim_unset($a, '1');
  dim_unset($a, 'foo');
  dim_unset($a, true);

  echo "======= dim_new_elem_set ===========================\n";
  try {
    $copy = $a;
    $copy[1][]['a'] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= dim_prop_get ===============================\n";
  try {
    var_dump($a[1]->foobaz[0]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= dim_prop_set ===============================\n";
  try {
    $copy = $a;
    $copy[1]->foobaz = vec[123];
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_cget_warn($a, $k) :mixed{
  try {
    var_dump($a[1][$k]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_cget($a, $k) :mixed{
  try {
    var_dump($a[1][$k] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_empty($a, $k) :mixed{
  try {
    var_dump(!($a[1][$k] ?? false));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_isset($a, $k) :mixed{
  try {
    var_dump(isset($a[1][$k]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}


function fini_set($a, $k) :mixed{
  try {
    $a[1][$k] = 123;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_incdec($a, $k) :mixed{
  try {
    $a[1][$k]++;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_setop($a, $k) :mixed{
  try {
    $a[1][$k] .= "some-str";
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_unset($a, $k) :mixed{
  try {
    unset($a[1][$k]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini($a) :mixed{
  echo "======= fini_cget_warn =============================\n";
  fini_cget_warn($a, 1);
  fini_cget_warn($a, 3);
  fini_cget_warn($a, '1');
  fini_cget_warn($a, 'foo');
  fini_cget_warn($a, true);

  echo "======= fini_cget_warn =============================\n";
  fini_cget($a, 1);
  fini_cget($a, 3);
  fini_cget($a, '1');
  fini_cget($a, 'foo');
  fini_cget($a, true);

  echo "======= fini_empty =================================\n";
  fini_empty($a, 1);
  fini_empty($a, 3);
  fini_empty($a, '1');
  fini_empty($a, 'foo');
  fini_empty($a, true);

  echo "======= fini_isset =================================\n";
  fini_isset($a, 1);
  fini_isset($a, 3);
  fini_isset($a, '1');
  fini_isset($a, 'foo');
  fini_isset($a, true);

  echo "======= fini_set ===================================\n";
  fini_set($a, 1);
  fini_set($a, 3);
  fini_set($a, '1');
  fini_set($a, 'foo');
  fini_set($a, true);

  echo "======= fini_set_new_elem ==========================\n";
  try {
    $copy = $a;
    $copy[1][] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= fini_incdec ================================\n";
  fini_incdec($a, 1);
  fini_incdec($a, 3);
  fini_incdec($a, '1');
  fini_incdec($a, 'foo');
  fini_incdec($a, true);

  echo "======= fini_incdec_new_elem =======================\n";
  try {
    $copy = $a;
    $copy[1][]++;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= fini_setop =================================\n";
  fini_setop($a, 1);
  fini_setop($a, 3);
  fini_setop($a, '1');
  fini_setop($a, 'foo');
  fini_setop($a, true);

  echo "======= fini_setop_new_elem =======================\n";
  try {
    $copy = $a;
    $copy[1][] .= "some-str";
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= fini_unset =================================\n";
  fini_unset($a, 1);
  fini_unset($a, 3);
  fini_unset($a, '1');
  fini_unset($a, 'foo');
  fini_unset($a, true);

  echo "======= fini_prop_get ===============================\n";
  try {
    var_dump($a[1]->foobaz);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= fini_prop_set ===============================\n";
  try {
    $copy = $a;
    $copy[1]->foobaz = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}
<<__EntryPoint>> function main(): void {
base(dict[1 => dict['a' => 100], 2 => "", "1" => dict['a' => 500]]);
dim(vec[null, dict[1 => dict['a' => 100], 2 => "", "1" => dict['a' => 500]]]);
fini(vec[null, dict[1 => "abc", 2 => "", "1" => "def"]]);
}
