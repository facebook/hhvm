<?hh

class YieldedException extends Exception {
}
class ReflectedException extends Exception {
}
function throwYieldedException() {
  throw new YieldedException();
}
function gen() {
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
function main_2176() {
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
