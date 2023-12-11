<?hh

class foo {
  <<__DynamicallyCallable>>
  static function out($foo) :mixed{
    return strtoupper($foo);
  }
}
<<__DynamicallyCallable>>
function my_strtoupper($foo, $bar) :mixed{
  return strtoupper($foo);
}

// Closure in variable
<<__EntryPoint>> function main(): void {
$a = function ($s) { return strtoupper($s); };
ob_start($a);
echo 'closure in variable', "\n";
ob_end_flush();

// Object (array) in variable
$a = vec['foo', 'out'];
ob_start($a);
echo 'object in variable', "\n";
ob_end_flush();

// Object with static array
ob_start(vec['foo', 'out']);
echo 'object via static array', "\n";
ob_end_flush();

$a = my_strtoupper<>;
ob_start($a);
echo 'function via variable', "\n";
ob_end_flush();
}
