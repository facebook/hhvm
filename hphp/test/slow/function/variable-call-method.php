<?hh

function g() :mixed{
  $f = 'IntlChar::ord';
  var_dump($f(' '));

  $f = vec['IntlChar', 'ord'];
  var_dump($f('='));

  $o = new UConverter('utf-8', 'latin1');
  $f = vec[$o, 'convert'];
  var_dump($f('foo'));
}


<<__EntryPoint>>
function main_variable_call_method() :mixed{
g();
}
