<?hh

function no_return() {
  throw new Exception('heh');
}

function main1() {
  $foo = array(1,2,3);
  no_return(...$foo);
}

function main2() {
  $foo = array(1,2,3);
  call_user_func_array(fun('no_return'), $foo);
}


<<__EntryPoint>>
function main_fcall_noreturn() {
try { main1(); } catch (exception $e) { echo "o"; }
try { main2(); } catch (exception $f) { echo "k"; }
echo "\n";
}
