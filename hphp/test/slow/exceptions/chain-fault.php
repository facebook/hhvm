<?hh // strict

class LogExn extends Exception {
  function __construct($msg) {
    parent::__construct($msg);
    echo "Constructing\n";
  }

}

function thrower() {
  try {
    throw new LogExn('inner');
  } finally {
    throw new Exception('finally');
  }
}
<<__EntryPoint>>
function main() {
  try {
    echo "Calling thrower\n";
    thrower();
  } catch (Throwable $t) {
    echo "In catch\n";
    var_dump($t);
    unset($t);
  }
  echo "Leaving main\n";
}
