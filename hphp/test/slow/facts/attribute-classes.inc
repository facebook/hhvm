<?hh

interface IBaseForAttributeFiltering {}

class BaseClassForAttributeFiltering implements IBaseForAttributeFiltering {}

class NoArgAttr implements HH\ClassAttribute {}

class TwoArgAttr implements HH\ClassAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1)[] {}
}

class NoArgMethodAttr implements HH\MethodAttribute {}

class TwoArgMethodAttr implements HH\MethodAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1)[] {}
}

// Not included in the `IndexedMethodAttributes` list in `.hhvmconfig.hdf`
class DontIndexThisMethodAttr implements HH\MethodAttribute {
  public function __construct()[] {}
}

class NoArgFileAttr implements HH\FileAttribute {}

class OneArgFileAttr implements HH\FileAttribute {
  public function __construct(dynamic $arg0)[] {}
}

class TwoArgFileAttr implements HH\FileAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1)[] {}
}

<<file: NoArgFileAttr, OneArgFileAttr(7), TwoArgFileAttr("Hello", "There")>>

<<TwoArgAttr("apple", "banana")>>
class AppleThenBanana extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("banana", "apple")>>
class BananaThenApple extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("apple", "carrot")>>
class AppleThenCarrot extends BaseClassForAttributeFiltering {}

<<NoArgAttr, TwoArgAttr("abc", "def")>>
class ClassWithTwoAttrs {}

class ClassWithMethodAttrs {

  <<NoArgMethodAttr>>
  public function methodWithNoArgAttr(): void {}

  <<TwoArgMethodAttr(42, "banana")>>
  public function methodWithTwoArgAttr(): void {}

  <<DontIndexThisMethodAttr>>
  public function dontIndexThisFunction(): void {}
}
