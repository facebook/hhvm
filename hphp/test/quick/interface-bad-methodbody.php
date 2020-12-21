<?hh
interface I {
  public function f()
  {
    echo 'This is illegal';
  }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
