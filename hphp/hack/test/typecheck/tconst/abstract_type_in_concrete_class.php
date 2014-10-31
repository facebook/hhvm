<?hh // strict

abstract class AbstractClassWithAbstract {
  abstract protected type const abstract_type = int;
}

class ClassWithAbstractType extends AbstractClassWithAbstract {
}
