<?hh

interface I {
  abstract const ctx C;
}

<<__DynamicallyCallable>> function f1(?I $a = null)[$a::C] :mixed{
  echo "in f1\n";
}

<<__DynamicallyCallable>> function f2(?I $a = null)[write_props, $a::C] :mixed{
  echo "in f2\n";
}

<<__DynamicallyCallable>> function pure($f)[] :mixed{ $f(); }
<<__DynamicallyCallable>> function rx($f)[rx] :mixed{ $f(); }
<<__DynamicallyCallable>> function defaults($f) :mixed{ $f(); }

<<__EntryPoint>>
function main() :mixed{
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
