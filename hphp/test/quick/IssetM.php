<?hh

function r($r) :mixed{
  print ($r ? " true" : "false")."\n";
}

function build() :mixed{
  $a = dict["A0" => 0,
             "A1" => "a1"];
  $b = dict["A" => $a,
             "B0" => 0,
             "B1" => "b1"];
  $c = dict["A" => $a,
             "B" => $b,
             "C0" => 0,
             "C1" => "c1"];

  return $c;
}

function main() :mixed{
  $arr = build();

  r(isset($arr["A"]));
  r(isset($arr["AX"]));
  r(isset($arr["B"]["A"]));
  r(isset($arr["X"]["A"]));
  r(isset($arr["B"]["X"]));
  r(isset($arr["X"]["X"]));
  r(isset($arr["X"]["X"]["X"]));
}
<<__EntryPoint>> function main_entry(): void {
print "Test begin\n";

main();

print "Test end\n";
}
