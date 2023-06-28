<?hh

function test1() :mixed{
  $a = new XMLReader();
  $a->expand(new stdClass);
}

<<__EntryPoint>>
function main(): void {
  $tests = vec[test1<>];
  foreach ($tests as $t) {
    try {
      $t();
    } catch (Exception $e) {
      echo $e->getMessage()."\n";
    }
  }
}
