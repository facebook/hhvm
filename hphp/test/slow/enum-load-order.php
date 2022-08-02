<?hh

abstract class AC {}

enum MyLovelyEnum1: classname<AC> {
  use MyLovelyEnum2;
}
enum MyLovelyEnum2: classname<AC> {
}

<<__EntryPoint>> function test(): void {
  echo "Done\n";
}
