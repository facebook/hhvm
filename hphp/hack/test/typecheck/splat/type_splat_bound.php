<?hh
<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function foo<T>((...T) $x):void { }
function bar<T>((function(int,...T):void) $f):void { }
