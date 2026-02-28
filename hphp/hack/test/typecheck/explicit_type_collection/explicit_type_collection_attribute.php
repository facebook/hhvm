<?hh
class A implements HH\ClassAttribute {
  public function __construct(public vec<mixed> $i) {}
}

<<A(vec<int>[1,2,3])>>
class foo{

}
