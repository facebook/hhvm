<?hh

class C<reify T> {}
class D extends C {}

<<__EntryPoint>>
function main() {
  new D();
}
