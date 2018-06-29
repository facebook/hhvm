<?hh // strict

/* Classic inheritance */
<<__PPL>>
class BaseClass {
}

<<__PPL>>
class ChildClass extends BaseClass {
}

<<__PPL>>
interface BaseInterface {
}

<<__PPL>>
class ChildClass2 implements BaseInterface {
}

<<__PPL>>
class ChildClass3 extends BaseClass implements BaseInterface {
}

/* Traits */
<<__PPL>>
trait MyTrait {
}

<<__PPL>>
class MyClass {
  use MyTrait;
}

/* Interfaces */
<<__PPL>>
interface ChildInterface extends BaseInterface {
}

/* Requires */
<<__PPL>>
trait MyTrait2 {
  require implements BaseInterface;
}

<<__PPL>>
abstract class RequireMe {
}

<<__PPL>>
interface BaseInterface2 {
  require extends RequireMe;
}
