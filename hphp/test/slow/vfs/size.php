<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(filesize(__DIR__.'/files/foo.dat'));
  var_dump(filesize(__DIR__.'/files/foo.txt'));
  var_dump(filesize(__DIR__.'/files/bar.dat'));
  var_dump(filesize(__DIR__.'/files/bar'));
  var_dump(filesize(__DIR__.'/files/bar/bar.dat'));
}
