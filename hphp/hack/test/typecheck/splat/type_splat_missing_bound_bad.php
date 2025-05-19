<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function ex1<T>(...T $_):void { }
function ex2<T>():(function(...T):void) { throw new Exception("A"); }
function ex3<T>((function(...T):void) $f):void { }
