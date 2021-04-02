<?hh

trait TImplementsIBase implements IBase {}

final class DerivedClass extends BaseClass implements IBase {
  use TImplementsIBase;
}

interface IRequireExtendsBaseClass {
  require extends BaseClass;
}

trait TRequireExtendsBaseClass {
  require extends BaseClass;
}

trait TRequireImplementsIBase {
  require implements IBase;
}

trait TRequireImplementsAndImplementsIBase implements IBase {
  require implements IBase;
}

trait TRequireExtendsBaseClassAndRequireImplementsIBase {
  require extends BaseClass;
  require implements IBase;
}

interface IimplementsIRequireExtendsBaseClass
  extends IRequireExtendsBaseClass {}

class UsesTRequireImplementsIBase implements IBase {
  use TRequireImplementsIBase;
}

abstract final class AbstractFinalClass {}
