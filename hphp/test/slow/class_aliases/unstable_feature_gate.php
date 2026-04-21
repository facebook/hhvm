<?hh
// Class alias syntax without enabling the unstable feature should error.

class Original {}

class Alias = Original;

<<__EntryPoint>>
function main(): void {
  echo "should not reach here\n";
}
