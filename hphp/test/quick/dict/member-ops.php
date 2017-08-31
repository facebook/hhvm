<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function base_elem_warn($d, $k) {
  try {
    var_dump($d[$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function base_elem($d, $k) {
  try {
    var_dump($d[$k]['a'] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function base_define($d, $k) {
  try {
    $d[$k]['a'] = 200;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($d);
}

function base_define_reffy($d, $k) {
  $str = 'abc';
  try {
    $d[$k]['a'] = &$str;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($d);
}

function base_unset($d, $k) {
  try {
    unset($d[$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($d);
}

function base($d) {
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

  echo "======= base_define_reffy ==========================\n";
  base_define_reffy($d, 1);
  base_define_reffy($d, 3);
  base_define_reffy($d, '1');
  base_define_reffy($d, 'foo');
  base_define_reffy($d, true);

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

  echo "======= base_str_to_arr ============================\n";
  try {
    $copy = $d;
    $copy[2][10] = 123;
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
    $copy->foobaz[0] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_elem_warn($a, $k) {
  try {
    var_dump($a[1][$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_elem($a, $k) {
  try {
    var_dump($a[1][$k]['a'] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function dim_define($a, $k) {
  try {
    $a[1][$k]['a'] = 200;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function dim_define_reffy($a, $k) {
  $str = 'abc';
  try {
    $a[1][$k]['a'] = &$str;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function dim_unset($a, $k) {
  try {
    unset($a[1][$k]['a']);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function dim($a) {
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

  echo "======= dim_define_reffy ===========================\n";
  dim_define_reffy($a, 1);
  dim_define_reffy($a, 3);
  dim_define_reffy($a, '1');
  dim_define_reffy($a, 'foo');
  dim_define_reffy($a, true);

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

  echo "======= dim_str_to_arr =============================\n";
  try {
    $copy = $a;
    $copy[1][2][10] = 123;
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
    $copy[1]->foobaz[0] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_cget_warn($a, $k) {
  try {
    var_dump($a[1][$k]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_cget($a, $k) {
  try {
    var_dump($a[1][$k] ?? "MISSING");
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_empty($a, $k) {
  try {
    var_dump(empty($a[1][$k]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_isset($a, $k) {
  try {
    var_dump(isset($a[1][$k]));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}

function fini_vget($a, $k) {
  try {
    $v = &$a[1][$k];
    var_dump($v);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_set($a, $k) {
  try {
    $a[1][$k] = 123;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_incdec($a, $k) {
  try {
    $a[1][$k]++;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_setop($a, $k) {
  try {
    $a[1][$k] .= "some-str";
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_unset($a, $k) {
  try {
    unset($a[1][$k]);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini_bind($a, $k) {
  $str = "some-str";
  try {
    $a[1][$k] = &$str;
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump($a);
}

function fini($a) {
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

  echo "======= fini_vget ==================================\n";
  fini_vget($a, 1);
  fini_vget($a, 3);
  fini_vget($a, '1');
  fini_vget($a, 'foo');
  fini_vget($a, true);

  echo "======= fini_vget_new_elem =========================\n";
  try {
    $copy = $a;
    $v = &$copy[1][];
    var_dump($v);
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

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

  echo "======= fini_bind ==================================\n";
  fini_bind($a, 1);
  fini_bind($a, 3);
  fini_bind($a, '1');
  fini_bind($a, 'foo');
  fini_bind($a, true);

  echo "======= fini_bind_new_elem =========================\n";
  try {
    $copy = $a;
    $str = "some-str";
    $copy[1][] = &$str;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "======= fini_str_to_arr ============================\n";
  try {
    $copy = $a;
    $copy[1][2][10] = 123;
    var_dump($copy);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

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

base(dict[1 => ['a' => 100], 2 => "", "1" => ['a' => 500]]);
dim([null, dict[1 => ['a' => 100], 2 => "", "1" => ['a' => 500]]]);
fini([null, dict[1 => "abc", 2 => "", "1" => "def"]]);
