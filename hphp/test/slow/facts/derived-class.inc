<?hh

trait TImplementsIBase implements IBase {}

final class DerivedClass extends BaseClass implements IBase {
  use TImplementsIBase;
}

abstract class AbstractDerived extends BaseClass {}

class NoFlagsDerived extends BaseClass {}

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

trait TRequireClassFinalClass {
  require class FinalClassUsesTRequireClass;
}

final class FinalClassUsesTRequireClass {
  use TRequireClassFinalCLass;
}

// `require class` trait constraints

trait TRequireClassFinalClass {
  require class FinalClassUsesTRequireClass;
}

final class FinalClassUsesTRequireClass {
  use TRequireClassFinalCLass;
}

trait TRequireClassFinalClassB {
  require class FinalClassUsesTRequireClassB;
  require extends SuperClassOfFinalClassB;
}

final class FinalClassUsesTRequireClassB extends SuperClassOfFinalClassB {
  use TRequireClassFinalCLass;
}
