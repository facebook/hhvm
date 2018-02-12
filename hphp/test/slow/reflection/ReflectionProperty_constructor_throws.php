<?hh

class C {}
try {
  new ReflectionProperty('CC', 'p');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
foreach (vec[null, '', 'p'] as $p) {
  try {
    new ReflectionProperty('C', $p);
  } catch (ReflectionException $e) {
    echo $e->getMessage() . "\n";
  }
}
$c = new C();
foreach (vec[null, '', 'p'] as $p) {
  try {
    new ReflectionProperty($c, $p);
  } catch (ReflectionException $e) {
    echo $e->getMessage() . "\n";
  }
}
