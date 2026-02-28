<?hh

trait T {
  const ctx C = [];
}

trait T1 {
  const ctx C = [zoned];
}

class Something {
  use T, T1; // ok
}

<<__EntryPoint>>
function main() :mixed{
  echo "ok\n";
}
