<?hh

type Something = int;
class Something {} // error, Something already exists

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
