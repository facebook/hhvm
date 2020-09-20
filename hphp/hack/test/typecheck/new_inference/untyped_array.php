<?hh // partial

function foo(array $m):void {
   $x = new Vector($m);
   $x[0]->foo();
   $y = new Map($m);
   $y['a']->foo();
}
