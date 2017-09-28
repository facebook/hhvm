<?hh // strict

function test(): void {
  $arr = array_map(
    $x ==> {
      if (false) {
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
