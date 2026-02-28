<?hh


<<__EntryPoint>>
function main_1284() :mixed{
  try {
    if ($a) $a == 0;
    print "done\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
