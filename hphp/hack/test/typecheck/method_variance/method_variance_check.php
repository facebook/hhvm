<?hh // strict

function bar0<+T>(): void {}
function bar1<-T>(): void {}
function bar2<+T as Foo>(): void {}
function bar3<-T, +T1>(): void {}
function bar4<-T, T1>(): void {}

// Don't expect error
function bar5<T>(): void {}
function bar6<T as Foo>(): void {}
function bar7<T, T1>(): void {}

class Foo {
  public function bar0<+T>(): void {}
  public function bar1<-T>(): void {}
  public function bar2<+T as Foo>(): void {}
  public function bar3<-T, +T1>(): void {}
  public function bar4<-T, T1>(): void {}

  // Don't expect error
  public function bar5<T>(): void {}
  public function bar6<T as Foo>(): void {}
  public function bar7<T, T1>(): void {}
}
