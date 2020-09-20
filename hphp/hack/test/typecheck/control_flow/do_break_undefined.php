<?hh // strict

function test_flow():string {
  $i = 0;
  do {
    if ($i < 5) break;
    $s = "hey";
    $i++;
  } while ($i < 10);
  // Because of the break statement $s isn't defined
  return $s;
}
