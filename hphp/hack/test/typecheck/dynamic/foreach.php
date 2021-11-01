<?hh

function testForEach(dynamic $x): void {
  foreach ($x as $id) {
    // $id : dynamic
    hh_show($id);
    $id->anyMethodCall();
  }
  // $x : dynamic
  hh_show($x);
}
