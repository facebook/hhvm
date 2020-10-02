<?hh // partial

function foo(varray $m):void {
   $x = new Vector($m);
   $x[0]->foo();
   $y = new Map($m);
   $y['a']->foo();
}
