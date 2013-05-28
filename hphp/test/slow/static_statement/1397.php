<?php

$static_var = 1;
  echo $static_var . "\n";
  static $static_var;
  echo $static_var . "\n";
  $static_var ++;
  echo $static_var . "\n";
