<?hh

trait T1 {
  require class C;
}

trait T2 {
  use T1;
}

class C {}

class D extends C {
  use T2;
}

<<__EntryPoint>>
function main(): void {
  new D();
  echo "done\n";
}
