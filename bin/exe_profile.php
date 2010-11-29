<?php

$server = $argv[1];
$password = @$argv[2];
$polling = isset($argv[3]) ? $argv[3] : 1;

$total = array();
$total[0] = $total[1] = $total[2] = $total[3] = $total[4] = 0;
while (true) {
  $ret = shell_exec("GET http://$server/prof-exe?auth=$password");
  $nums = json_decode($ret);
  for ($i = 0; $i < count($nums); $i++) {
    $num = $nums[$i];
    if ($num == -1) break;

    $total[$num] += $nums[++$i];
  }

  $sum = /* $total[1] + */ $total[2] + $total[3];

  $out = '';
  //$out .= sprintf("Server: %2d%%\t", (int)($total[1] * 100 / $sum));
  $out .= sprintf("Extension: %2d%%\t", (int)($total[2] * 100 / $sum));
  $out .= sprintf("User: %2d%%\n", (int)($total[3] * 100 / $sum));
  echo $out;

  sleep($polling);
}
