<?hh

function takes(~?int $i): void {
  var_dump($i);
}

<<__EntryPoint>>
function main(): void {
  takes(null);
}
