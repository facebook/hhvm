<?hh

enum class MyClass: string {
  string KeyedByIC = 'KeyedByIC';
}

<<__Memoize(MyClass#KeyedByIC)>>
function foo(): void {}
