<?hh

class C {}

<<__EntryPoint>>
function main_reflection_property_constructor_throws() :mixed{
try {
  new ReflectionProperty('CC', 'p');
} catch (ReflectionException $e) {
  echo $e->getMessage() . "\n";
}
foreach (vec[null, '', 'p'] as $p) {
  try {
    new ReflectionProperty('C', $p);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
$c = new C();
foreach (vec[null, '', 'p'] as $p) {
  try {
    new ReflectionProperty($c, $p);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
}
