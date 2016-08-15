<?php
$x = date_create();
foreach (hh\objprof_get_strings(1) as $k => $v) {
  $t .= $k;
}
var_dump(strlen($t));
