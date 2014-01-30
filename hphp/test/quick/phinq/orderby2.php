<?php

include "queryable.inc";

$orders = new Queryable();

$q = from $o in $orders
     orderby $o->Customer->Name, $o->Total ascending
     select $o;

foreach ($q as $e) {
  print_result($e);
}

$q2 = from $o in $orders
     orderby $o->Customer->Name, $o->Total
     select $o;

foreach ($q as $e) {
  print_result($e);
}
