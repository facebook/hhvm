<?php

function heh() {
  $a = array('x'=>1,'y'=>2,'z'=>3,'k'=>4);
  $b = $a;

  extract($a, EXTR_REFS);
  $vars = get_defined_vars();
  ksort($vars);
  var_dump($vars);

  $a['y'] = 42;
  var_dump($a['y']);
  var_dump($b['y']);
  var_dump($y);
}

function extract_empty_ref() {
    $a = array();
    extract($a, EXTR_REFS);

    var_dump('OK');
}

heh();
extract_empty_ref();
