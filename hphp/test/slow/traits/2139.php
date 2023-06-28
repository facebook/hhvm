<?hh

trait T {
  abstract const ctx C1;
}

interface I {
  const ctx C1 = [];
}

class Something implements I {
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  echo "ok\n";
}
