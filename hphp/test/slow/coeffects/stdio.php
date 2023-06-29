<?hh

<<__EntryPoint>>
function stdio_test()[]: void {
  invariant(HH\stdin() is resource, "oops");
  echo "ok\n";
}
