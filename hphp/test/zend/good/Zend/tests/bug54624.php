<?hh
class A
{
    function foo(A $param) {
    }
}

class_alias('A', 'AliasA');

<<__EntryPoint>> function main(): void {
  eval('
    class B extends A
    {
        function foo(AliasA $param) {
        }
    }
  ');

  echo "DONE\n";
}
