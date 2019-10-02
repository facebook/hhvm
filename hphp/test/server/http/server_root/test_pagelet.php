<?hh

function main() {
  $r = pagelet_server_task_start("pagelet.php");
  var_dump($r);
  do {
    var_dump($res = pagelet_server_task_result($r, inout $h, inout $c));
  } while ($res !== null && $c === 0);
  var_dump($c);
}

main();
