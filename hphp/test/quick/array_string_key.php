<?hh

<<__EntryPoint>> function main(): void {
  // test CGetM
  $a = dict[];
  $a[0] = "one";
  try { echo $a["0"]; } catch (Exception $e) { echo $e->getMessage()."\n"; }

  // test SetM
  $a = dict[];
  $a["0"] = "two";
  try { echo $a[0]; } catch (Exception $e) { echo $e->getMessage()."\n"; }

  // test IssetM
  $a = vec["narf"];
  echo (string)(isset($a["0"])) . "\n";

  // test UnsetM
  $a = dict[];
  $a[0] = "uh oh";
  unset($a["0"]);
  echo count($a) . "\n";

  // make sure normal strings work
  $a = dict[];
  $a["not an int"] = "woo";
  echo $a["not an int"] . "\n";

  // it has to be strictly an integer
  $a = dict[];
  $a[0] = "hoo!";
  $a["00"] = "this is different";
  $a["0 "] = "and this";
  echo $a[0] . "\n";
}
