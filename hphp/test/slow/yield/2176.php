<?hh

class YieldedException extends Exception {
}
class ReflectedException extends Exception {
}
function throwYieldedException() :mixed{
  throw new YieldedException();
}
function gen() :AsyncGenerator<mixed,mixed,void>{
  try {
    $a = yield throwYieldedException();
    echo 'Gen got '.$a;
  }
 catch (YieldedException $e) {
    var_dump('Got yieldedException, re-raising.');
    throw $e;
  }
 catch (ReflectedException $e) {
    var_dump('Got Reflected Exception');
  }
}

<<__EntryPoint>>
function main_2176() :mixed{
try {
  $g = gen();
  $g->next();
}
 catch(YieldedException $e) {
  try {
    $g->raise(new ReflectedException());
  }
 catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
}
