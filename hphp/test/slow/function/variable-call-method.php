<?hh

function g() {
  $f = 'IntlChar::ord';
  var_dump($f(' '));

  $f = varray['IntlChar', 'ord'];
  var_dump($f('='));

  $o = new UConverter('utf-8', 'latin1');
  $f = varray[$o, 'convert'];
  var_dump($f('foo'));
}


<<__EntryPoint>>
function main_variable_call_method() {
g();
}
