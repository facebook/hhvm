<?php

print "Test begin\n";

function r($r) {
  print ($r ? " true" : "false")."\n";
}

function build() {
  $a = array("A0" => 0,
             "A1" => "a1");
  $b = array("A" => $a,
             "B0" => 0,
             "B1" => "b1");
  $c = array("A" => $a,
             "B" => $b,
             "C0" => 0,
             "C1" => "c1");

  return $c;
}

function main() {
  $arr = build();

  r(isset($arr["A"]));
  r(isset($arr["AX"]));
  r(isset($arr["B"]["A"]));
  r(isset($arr["X"]["A"]));
  r(isset($arr["B"]["X"]));
  r(isset($arr["X"]["X"]));
  r(isset($arr["X"]["X"]["X"]));
}
main();

print "Test end\n";
