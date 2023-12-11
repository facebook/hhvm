<?hh
function foo(stdClass $a, AnyArray $b, callable $c, ?stdClass $d = null,
             $e = null, string $f, bool $g, int $h, float $i,
             NotExisting $j) :mixed{ }
function bar(): stdClass { return new stdClass; }

<<__EntryPoint>>
function main_reflection_type_detailed() :mixed{
$closure = function (Test $a): Test { return $a; };
echo "*** functions\n";
foreach (vec[
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
foreach (vec[
  new ReflectionMethod('SplObserver', 'update'),
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
foreach (vec[
  new ReflectionMethod('SplObserver', 'update'),
  new ReflectionFunction('bar'),
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
}
