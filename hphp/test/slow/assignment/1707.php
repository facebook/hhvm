<?php

function g($key, $old, $new, $s = false) {
  $diff = array();
  if ($old !== $new) {
    if ($s) {
      $old = f($old, true);
      $new = f($new, true);
    }
    $diff['old'][$key] = $old;
    $diff['new'][$key] = $new;
  }
  return $diff;
}
function f($a0, $a1) {
  return 'should_be_modified';
}
var_dump(g('key', 'old', 'new', true));
