<?hh

function one($a) :mixed{ echo "one\n"; }
function two($a, $b) :mixed{ echo "two\n"; }
function three($a, $b, $c) :mixed{ echo "three\n"; }

function error_handler($errno, $errstr) :mixed{
  throw new Exception("$errno, $errstr");
}

<<__EntryPoint>> function main(): void {
  one(1);
  try { two(1); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { three(1); } catch (Exception $e) { var_dump($e->getMessage()); }
}
