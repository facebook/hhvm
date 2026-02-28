<?hh

type TF = (readonly function(int):string);

function foo(<<__IgnoreReadonlyError>> TF $f):void { }
