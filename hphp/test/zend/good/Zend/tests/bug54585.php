<?hh
function testing($source) {
  unset($source['']);
}
<<__EntryPoint>>
function main(): void {
  testing($_GET);
  echo "ok\n";
}
