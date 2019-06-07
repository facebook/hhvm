<?hh

class foo {
  public function __toString() { throw new Exception("asd"); }
}

function main() {
  echo (string)(new foo());
}
<<__EntryPoint>> function main_entry() {
try { main(); } catch (Exception $x) {}
echo "done.\n";
}
