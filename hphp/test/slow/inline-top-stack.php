<?hh

function dump() {
  foreach ($GLOBALS['exns'] as $f => $exn) {
    $trace = implode(
      ', ',
      array_map($x ==> $x['function'].':'.$x['line'], $exn->getTrace())
    );
    echo "$f: $trace\n";
  }
}

<<__ALWAYS_INLINE>>
function red($a) {
  $a['exns'][__FUNCTION__] = new Exception;
}

<<__ALWAYS_INLINE>>
function green($a) {
  $a['exns'][__FUNCTION__] = new Exception;
  red($a);
}

<<__ALWAYS_INLINE>>
function blue($a) {
  $a['exns'][__FUNCTION__] = new Exception;
  green($a);
}

function main($a) {
  blue($a);
}

for ($i = 0; $i < 10; $i++) main($GLOBALS);
dump();
