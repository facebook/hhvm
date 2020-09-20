<?hh // strict

class A implements HH\ClassAttribute {
  public function __construct(shape("A" => dict<int,vec<string>>) $x) {}
}

<<A(shape("A" => dict[123 => vec[MyClass::CLASS_CONST]]))>>
class X {}
