<?hh

<<file: __EnableUnstableFeatures('class_type')>>

final class MyClass {}

function test(class<MyClass> $cp): void {
  if ($cp) {
  }
}
