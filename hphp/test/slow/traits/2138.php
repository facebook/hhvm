<?hh

trait T {
  const ctx C = [];
}

trait T1 {
  const ctx C = [policied];
}

class Something {
  use T, T1; // ok
}

<<__EntryPoint>>
function main() {
  echo "ok\n";
}
