<?

function funcname() {
  return "funcname";
}

function otherfuncname() {
  return "funcname";
}


$nm = funcname();
echo $nm() . "\n";

for ($i = 0; $i < 10000000; $i++) {
  $nm = $nm();
  if ($i % 100 == 0) {
    $nm = "otherfuncname";
  }
}

echo $nm() . "\n";

$pf="printf";
$pf("printf results: %12s\n", "mcguffin");
