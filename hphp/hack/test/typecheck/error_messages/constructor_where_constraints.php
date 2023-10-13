<?hh

class X {}

class A<T> {
  public function __construct(classname<T> $f) where T = nothing
  {}
}

function test():void {
  new A<X>(X::class);
}
