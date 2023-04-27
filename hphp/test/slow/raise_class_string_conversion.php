<?hh

class Atype {}

function set(dict<string, string> $cache) {
  $cache[Atype::class] = "Atype";
}

<<__EntryPoint>>
function main(): void {
  set(dict[]);
  echo "Done.\n";
}
