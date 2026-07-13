<?hh

<<__EntryPoint>>
function main() :mixed{
  $r = pagelet_server_task_start("pagelet.php");
  var_dump($r);
  do {
    $res = pagelet_server_task_result($r, inout $h, inout $c);
    var_dump($res);
  } while ($res !== null && $c === 0);
  try {
    var_dump($c);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
