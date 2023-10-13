<?hh

class Bar {}

<<__Memoize>>
function some_function((int, Bar, string) $tup): void {}
