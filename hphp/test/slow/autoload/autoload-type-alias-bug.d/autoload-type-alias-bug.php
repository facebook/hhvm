<?hh

function take(Bar $_): void {
  echo "Done\n";
}

<<__EntryPoint>>
function main_autoload_type_alias_bug() :mixed{
  take(new Baz());
}
