<?hh

function test() {
  $a = IntlTimeZone::createDefault();
  $a->hasSameRules(new stdClass);
}

<<__EntryPoint>>
function main(): void {
  try {
    test();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
