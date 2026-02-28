<?hh
interface I {
  public function f()
:mixed  {
    echo 'This is illegal';
  }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
