<?hh
final class InlineTopStack { public static $storage = Map { 'exns' => Map {} }; /* no COW */ }
function dump() {
  foreach (InlineTopStack::$storage['exns'] as $f => $exn) {
    $trace = implode(
      ', ',
      array_map($x ==> $x['function'].':'.($x['line'] ?? 'entrypoint'), ($exn as Exception)->getTrace()),
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
  $a['exns'] = Map {__FUNCTION__ => new Exception };
  green($a);
}
<<__EntryPoint>>
function main_top_stack() {
  for ($i = 0; $i < 10; $i++) blue(InlineTopStack::$storage);
  dump();
}
