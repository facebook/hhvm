<?hh

function non_rx($fn) {
  if ($fn) $fn(null);
}

<<__RxLocal>>
function rx_local($fn) {
  if ($fn) $fn(null);
}

<<__RxShallow>>
function rx_shallow($fn) {
  if ($fn) $fn(null);
}

<<__Rx>>
function rx($fn) {
  if ($fn) $fn(null);
}

<<__Pure>>
function pure($fn) {
  if ($fn) $fn(null);
}

<<__EntryPoint>>
function main() {
  $functions = vec['non_rx', 'rx_local', 'rx_shallow', 'rx', 'pure'];
  foreach ($functions as $caller) {
    foreach ($functions as $callee) {
      try {
        $caller($callee);
        echo "$caller -> $callee: ok\n";
      } catch (Exception $e) {
        echo "$caller -> $callee: ".$e->getMessage()."\n";
      }
    }
  }
}
