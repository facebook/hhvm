<?hh


function foo<T>((...T) $x):void { }
function bar<T>((function(int,...T):void) $f):void { }
