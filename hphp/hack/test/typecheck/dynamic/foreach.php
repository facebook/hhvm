<?hh // partial

function testForEach(dynamic $x, $y): void {
  foreach ($x as $id) {
    // $id : dynamic
    hh_show($id);
    $id->anyMethodCall();
  }
  // $x : dynamic
  hh_show($x);
}
