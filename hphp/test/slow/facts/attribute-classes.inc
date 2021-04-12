<?hh

interface IBaseForAttributeFiltering {}

class BaseClassForAttributeFiltering implements IBaseForAttributeFiltering {}

class NoArgAttr implements HH\ClassAttribute {}

class TwoArgAttr implements HH\ClassAttribute {
  public function __construct(dynamic $arg0, dynamic $arg1) {}
}

<<TwoArgAttr("apple", "banana")>>
class AppleThenBanana extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("banana", "apple")>>
class BananaThenApple extends BaseClassForAttributeFiltering {}

<<TwoArgAttr("apple", "carrot")>>
class AppleThenCarrot extends BaseClassForAttributeFiltering {}

<<NoArgAttr, TwoArgAttr("abc", "def")>>
class ClassWithTwoAttrs {}
