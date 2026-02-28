<?hh

function no_return() :mixed{
  throw new Exception('heh');
}

function main1() :mixed{
  $foo = vec[1,2,3];
  no_return(...$foo);
}

function main2() :mixed{
  $foo = vec[1,2,3];
  call_user_func_array(no_return<>, $foo);
}


<<__EntryPoint>>
function main_fcall_noreturn() :mixed{
try { main1(); } catch (Exception $e) { echo "o"; }
try { main2(); } catch (Exception $f) { echo "k"; }
echo "\n";
}
