<?hh

class test {}


<<__EntryPoint>>
function main_reflection_class_get_static_property_value() :mixed{
$c = new ReflectionClass('test');
try {
  var_dump($c->getStaticPropertyValue('notfound', 'default'));
  var_dump($c->getStaticPropertyValue('notfound', null));
  var_dump($c->getStaticPropertyValue('notfound'));
} catch (ReflectionException $e) {
  echo $e->getMessage(), PHP_EOL;
}
}
