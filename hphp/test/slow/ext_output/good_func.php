<?php

function good_func() {
  echo 'this doen\'t print';
  return 'nor this';
}
ob_start('good_func');
echo 'this doen\'t print';
var_dump(ob_end_clean());
