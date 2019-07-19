<?hh

class FuncAttr1 implements HH\FunctionAttribute {
  public function __construct(public string $str) {}
}

class FuncAttr2 implements HH\FunctionAttribute {
  public function __construct(public string $str) {}
}

class ClassAttr1 implements HH\ClassAttribute {
  public function __construct(public string $str) {}
}

class ClassAttr2 implements HH\ClassAttribute {
  public function __construct(public string $str) {}
}

class MethodAttr1 implements HH\MethodAttribute {
  public function __construct(public string $str) {}
}

class MethodAttr2 implements HH\MethodAttribute {
  public function __construct(public string $str) {}
}

class ParamAttr1 implements HH\ParameterAttribute {
  public function __construct(public string $str) {}
}

class ParamAttr2 implements HH\ParameterAttribute {
  public function __construct(public string $str) {}
}

@ClassAttr1('before') @ClassAttr2('i')
class C {
  @MethodAttr1('even') @MethodAttr2('turn')
  public function f(
    @ParamAttr1('the') @ParamAttr2('key')
    string $_,
  ): void {}
}

@ClassAttr1
class D {}

@__Memoize
function f(): @__Soft int {
  echo "f() called";
  return "hello";
}

@FuncAttr1('hello') @FuncAttr2('world')
function reflect(): void {
  $rf = new ReflectionFunction('reflect');
  var_dump($rf->getAttributeClass(FuncAttr1::class)->str);
  var_dump($rf->getAttributeClass(FuncAttr2::class)->str);

  $rc = new ReflectionClass('C');
  var_dump($rc->getAttributeClass(ClassAttr1::class)->str);
  var_dump($rc->getAttributeClass(ClassAttr2::class)->str);

  $rm = new ReflectionMethod('C', 'f');
  var_dump($rm->getAttributeClass(MethodAttr1::class)->str);
  var_dump($rm->getAttributeClass(MethodAttr2::class)->str);

  $rp = $rm->getParameters()[0];
  var_dump($rp->getAttributeClass(ParamAttr1::class)->str);
  var_dump($rp->getAttributeClass(ParamAttr2::class)->str);
}

@__EntryPoint
function main(): void {
  f();
  f();
  reflect();
}
