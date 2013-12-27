<?php

include "queryable.inc";

function let() { return "ok\n"; }

$orders = new Queryable();

$q = from $o in $orders
     let $t = $o->Details.Sum(
       function ($d) { return $d->UnitPrice * $d->Quantity; }
     )
     where $t >= 1000
     select Map {"OrderID" => $o->OrderID, "Total" => $t };

foreach ($q as $e) {
  echo $e."\n";
}

echo let();
