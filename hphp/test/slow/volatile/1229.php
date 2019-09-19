<?hh

function autoload_first($name) {
  echo __METHOD__ . "\n";
  throw new Exception('first', 0, new Exception('first_inner'));
}
function autoload_second($name) {
  echo __METHOD__ . "\n";
  throw new Exception('second', 0, new Exception('second_inner'));
}
function __autoload($name) {
  echo __METHOD__ . "\n";
  throw new Exception('__autoload');
}

<<__EntryPoint>>
function main_1229() {
  spl_autoload_register('autoload_first');
  spl_autoload_register('autoload_second');
  try {
    var_dump(class_exists('A'));
  } catch(Exception $e) {
    do {
      echo $e->getMessage() . "\n";
    } while($e = $e->getPrevious());
  }
  try {
    $obj = new A();
  }
  catch(Exception $e) {
    do {
      echo $e->getMessage() . "\n";
    } while($e = $e->getPrevious());
  }
}
