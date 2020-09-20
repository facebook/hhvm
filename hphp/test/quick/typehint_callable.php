<?hh

function a(callable $b) { $b(); }
<<__EntryPoint>> function main(): void {
$c = function() { var_dump(true); };
a($c);

try {
  a('hi');
} catch (Exception $e) {
  var_dump($e);
}
}
