<?hh

function test1() {
  $a = new XMLReader();
  $a->expand(new stdclass);
}

<<__EntryPoint>>
function main(): void {
  $tests = vec[fun('test1')];
  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
