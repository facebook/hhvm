<?hh

trait T {
  const ctx C = [];
}

interface I {
  const ctx C = [];
}

class Something implements I {
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  echo "ok\n";
}
