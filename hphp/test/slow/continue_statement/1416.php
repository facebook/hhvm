<?hh


<<__EntryPoint>>
function main_1416() :mixed{
  for ($i = 0; $i < 3; $i++) {
    echo "Start Of I loop\n";
    $continue_after_inner_loop = false;
    for ($j = 0; !$continue_after_inner_loop; $j++) {
      if ($j >= 2) {
        $continue_after_inner_loop = true;
        continue;
      }
      echo "I : $i J : $j"."\n";
    }
    if ($continue_after_inner_loop) continue;
    echo "End\n";
  }
  for ($i = 0; $i < 10; $i++) {
    if ($i % 2 == 0) continue;
    echo $i."\n";
  }
  for ($i = 0; $i < 10; $i++) {
    if ($i % 2 == 0) continue;
    echo $i."\n";
  }
}
