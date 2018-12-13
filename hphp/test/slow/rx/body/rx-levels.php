<?hh

<<__Rx>>
function full_rx() {
  echo "Full\n";
}

<<__RxShallow>>
function shallow_rx() {
  full_rx();
  echo "Shallow\n";
}

<<__RxLocal>>
function local_rx() {
  shallow_rx();
  echo "Local\n";
}

<<__EntryPoint>>
function main() {
  local_rx();
}
