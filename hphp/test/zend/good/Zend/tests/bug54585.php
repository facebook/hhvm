<?hh
function testing($source) :mixed{
  unset($source['']);
}
<<__EntryPoint>>
function main(): void {
  testing(\HH\global_get('_GET'));
  echo "ok\n";
}
