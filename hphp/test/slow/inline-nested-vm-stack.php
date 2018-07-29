<?hh

class Guard {
  function __construct(private string $f) {}
  function __destruct() {
    $trace = implode(
      ', ',
      array_map($x ==> $x['function'].':'.$x['line'], debug_backtrace())
    );
    echo $this->f.": $trace\n";
  }
}

<<__ALWAYS_INLINE>>
function red() {
  $a = new Guard(__FUNCTION__);
}

<<__ALWAYS_INLINE>>
function green() {
  $a = new Guard(__FUNCTION__);
  red();
}

<<__ALWAYS_INLINE>>
function blue() {
  $a = new Guard(__FUNCTION__);
  green();
}

function main() {
  blue();
}

for ($i = 0; $i < 10; $i++) main();
