<?hh
function testing($source) :mixed{
  unset($source['']);
}
<<__EntryPoint>>
function main(): void {
  testing($_GET);
  echo "ok\n";
}
