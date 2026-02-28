<?hh

abstract class AbstractClassWithAbstract {
  abstract const type abstract_type as int;
}

class ClassWithAbstractType extends AbstractClassWithAbstract {}
