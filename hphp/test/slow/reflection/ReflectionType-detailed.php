<?hh
function foo(stdClass $a, array $b, callable $c, stdClass $d = null,
             $e = null, string $f, bool $g, int $h, float $i,
             NotExisting $j) { }
function bar(): stdClass { return new stdClass; }
class c extends stdClass {
  function bar(self $x): int { return 1; }
  function pbar(parent $x): int { return 1; }
  function factory(): self { return new c; }
  function pfactory(): parent { return new stdClass; }
}
$closure = function (Test $a): Test { return $a; };
echo "*** functions\n";
foreach ([
  new ReflectionFunction('foo'),
  new ReflectionFunction($closure),
] as $idx => $rf) {
  foreach ($rf->getParameters() as $idx2 => $rp) {
    echo "** Function $idx - Parameter $idx2\n";
    var_dump($rp->hasType());
    $ra = $rp->getType();
    if ($ra) {
      var_dump($ra->allowsNull());
      var_dump($ra->isBuiltin());
      var_dump((string)$ra);
    }
  }
}
echo "\n*** methods\n";
foreach ([
  new ReflectionMethod('SplObserver', 'update'),
  new ReflectionMethod('c', 'bar'),
  new ReflectionMethod('c', 'pbar'),
  new ReflectionMethod($closure, '__invoke'),
] as $idx => $rm) {
  foreach ($rm->getParameters() as $idx2 => $rp) {
    echo "** Method $idx - parameter $idx2\n";
    var_dump($rp->hasType());
    $ra = $rp->getType();
    if ($ra) {
      var_dump($ra->allowsNull());
      var_dump($ra->isBuiltin());
      var_dump((string)$ra);
    }
  }
}
echo "\n*** return types\n";
foreach ([
  new ReflectionMethod('SplObserver', 'update'),
  new ReflectionFunction('bar'),
  new ReflectionMethod('c', 'bar'),
  new ReflectionMethod('c', 'factory'),
  new ReflectionMethod('c', 'pfactory'),
  new ReflectionFunction($closure),
  new ReflectionMethod($closure, '__invoke'),
] as $idx => $rf) {
  echo "** Function/method return type $idx\n";
  var_dump($rf->hasReturnType());
  $ra = $rf->getReturnType();
  if ($ra) {
    var_dump($ra->allowsNull());
    var_dump($ra->isBuiltin());
    var_dump((string)$ra);
  }
}
