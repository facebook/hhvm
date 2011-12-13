<?php

print "Test begin\n";

class C {
  const x = "C::x constant";
}

var_dump(C::x);
$c = "C";
var_dump($c::x);

print "Test end\n";
