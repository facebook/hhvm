<?hh // strict

function test(bool $b): void {
  $arr = array_map(
    $x ==> {
      if ($b) {
        return $x;
      }
    },
    array('test'),
  );
  hh_show($arr);
  foreach ($arr as $x) {
    if ($x) {
    }
  }
}
