<?hh

class Destruct {
  function __destruct() {}
}

class OptionalDestruct {
  <<__OptionalDestruct>>
  function __destruct() { echo "OptionalDestruct destructing\n"; }
}

$o = new Redis();
echo "Created Redis\n";
$o = new OptionalDestruct();
echo "Created OptionalDestruct\n";
unset($o);
new Destruct();
