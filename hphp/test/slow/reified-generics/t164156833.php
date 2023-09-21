<?hh

newtype NT<T> = string;

class C1<reify T> { }
class C2<reify T> extends C1<T> { }

<<__EntryPoint>>
function main(): void {
  new C2<NT<int>>();
  new C2<NT<float>>();
  echo "Done\n";
}
