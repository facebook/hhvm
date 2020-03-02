<?hh // strict

function test(bool $b): void {
  $arr = array_map(
    $x ==> {
      if ($b) {
        return $x;
      }
    },
    varray['test'],
  );
  foreach ($arr as $x) {
    if ($x) {
    }
  }
}
