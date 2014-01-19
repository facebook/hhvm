<?php

function test($tr_data) {
  $temp_tr = null;
  foreach ($tr_data as $tr_id => $tr_row) {
    if ($tr_row == 45) $temp_tr = $tr_id;
    if ($tr_id == 0) {
      continue;
    }
 else {
      return $tr_row;
    }
  }
  if ($temp_tr) {
    return $temp_tr;
  }
  return null;
}
var_dump(test(array('a' => 1, 'b' => 45)));
