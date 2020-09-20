<?hh

class asd {
  static $PROP = 2;
}


<<__EntryPoint>>
function main_reflection_sprop() {
$y = (new ReflectionClass('asd'))->getProperty('PROP');
$y->setValue('asd');
var_dump(asd::$PROP);
}
