<?php

function setAttribute() {
  if (($v_size = func_num_args()) == 0) {
    return true;
  }
  $v_att_list = &func_get_args();
  return true;
}

<<__EntryPoint>>
function main_1312() {
setAttribute('a');
}
