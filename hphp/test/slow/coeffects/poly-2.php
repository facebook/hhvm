<?hh

interface I {
  abstract const ctx C;
}

function f1(I $a = null)[$a::C] {
  echo "in f1\n";
}

function f2(I $a = null)[write_props, $a::C] {
  echo "in f2\n";
}

function pure($f)[] { $f(); }
function rx($f)[rx] { $f(); }
function defaults($f) { $f(); }

<<__EntryPoint>>
function main() {
  $callers = vec['pure', 'rx', 'defaults'];
  $callees = vec['f1', 'f2'];
  foreach ($callers as $caller) {
    echo "=== $caller ===\n";
    foreach ($callees as $callee) {
      $caller($callee);
      echo "$callee: ok\n";
    }
  }
}
