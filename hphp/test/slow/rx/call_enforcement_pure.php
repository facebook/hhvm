<?hh

function non_rx($fn) {
  if ($fn) $fn(null);
}

function rx_local($fn)[rx_local] {
  if ($fn) $fn(null);
}

function rx_shallow($fn)[rx_shallow] {
  if ($fn) $fn(null);
}

function rx($fn)[rx] {
  if ($fn) $fn(null);
}

function pure($fn)[] {
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
