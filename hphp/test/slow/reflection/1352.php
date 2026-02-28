<?hh

function show_params($params) :mixed{
  echo 'parameters => array(' . count($params) . ") {\n";
  foreach ($params as $p) {
    echo "  name => {$p->getName()}\n";
  }
  echo "}\n";
}
function show($rf) :mixed{
  var_dump($rf->getName());
  var_dump($rf->isUserDefined());
  var_dump($rf->getStartLine());
  var_dump($rf->getEndLine());
  var_dump($rf->getDocComment());
  var_dump($rf->getFileName() === __FILE__);
  show_params($rf->getParameters());
  var_dump($rf->getNumberOfParameters());
  var_dump($rf->getNumberOfRequiredParameters());
}
<<__EntryPoint>>
function entrypoint_1352(): void {

  $f = function() {
    echo "in \$f\n";
  };
  $g = function($a = 'default') {
    echo "in \$g; passed $a\n";
  };

  /** This doc comment is so helpful and descriptive. */
  $add = function($x, $y) {
    $sum = $x + $y;
    echo 'x + y = ' . $sum . "\n";
  };

  $rf = new ReflectionFunction($f);
  $rg = new ReflectionFunction($g);
  $radd = new ReflectionFunction($add);

  echo "invoking f\n";
  $rf->invoke();

  echo "\ninvoking g\n";
  $rg->invoke('hello');
  $rg->invokeArgs(vec['goodbye']);

  echo "\ninvoking add\n";
  $radd->invoke(1, 2);
  $radd->invokeArgs(vec[5000000000, 5000000000]);

  echo "\nshowing f\n";
  show($rf);

  echo "\nshowing g\n";
  show($rg);

  echo "\nshowing add\n";
  show($radd);
}
