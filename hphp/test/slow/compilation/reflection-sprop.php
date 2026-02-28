<?hh

class asd {
  public static $PROP = 2;
}


<<__EntryPoint>>
function main_reflection_sprop() :mixed{
$y = (new ReflectionClass('asd'))->getProperty('PROP');
$y->setValue('asd');
var_dump(asd::$PROP);
}
