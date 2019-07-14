<?hh

trait T {
  <<__LateInit>> public property $p;
};

class C {
  use T;
}
<<__EntryPoint>> function main(): void {
echo "OK\n";
}
