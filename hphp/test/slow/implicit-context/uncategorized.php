<?hh

abstract class MyClass {
  <<__MemoizeLSB(#Uncategorized)>>
  public static function do(): void {
  }
}

<<__Memoize(#Uncategorized)>>
function myfoo(): void {
}

<<__EntryPoint>>
function mymain(): void {
  echo (new ReflectionFunction('myfoo'))->getAttribute('__Memoize')[0] === HH\MemoizeOption#Uncategorized
    ? "OK\n" : "Unequal\n";
  echo (new ReflectionMethod('MyClass', 'do'))->getAttribute('__MemoizeLSB')[0] === HH\MemoizeOption#Uncategorized
    ? "OK\n" : "Unequal\n";
}
