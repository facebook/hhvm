<?hh

function check($name) :mixed{
  $o = new $name('now', new DateTimeZone('UTC'));
  var_dump(is_varray($o->__sleep()));
  $s = serialize($o);
  $o2 = unserialize($s);
  var_dump($o2 == $o);
  var_dump(isset($o2->_date_time) === false);
}

class A extends DateTime {}

<<__EntryPoint>>
function main_serialize() :mixed{
check('DateTime');
check('A');
}
