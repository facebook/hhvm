<?php

include "queryable.inc";

function let() { return "ok\n"; }

$orders = new Queryable();

$q = from $o in $orders
     from $d in $o->Details
     let $t = sum($d->UnitPrice * $d->Quantity)
     where $t >= 1000
     select Map {"OrderID" => $o->OrderID, "Total" => $t };

foreach ($q as $e) {
  print_result($e);
}

echo let();
