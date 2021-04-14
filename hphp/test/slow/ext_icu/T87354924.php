<?hh

function test() {
  $a = IntlTimeZone::createDefault();
  $a->hasSameRules(new stdclass);
}

<<__EntryPoint>>
function main(): void {
  try {
    test();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
