<?hh

class foo {
  public function __toString() :mixed{ throw new Exception("asd"); }
}

function main() :mixed{
  echo (string)(new foo());
}
<<__EntryPoint>> function main_entry() :mixed{
try { main(); } catch (Exception $x) {}
echo "done.\n";
}
