<?hh

class C<reify T> {}
class D extends C {}

<<__EntryPoint>>
function main() :mixed{
  new D();
}
