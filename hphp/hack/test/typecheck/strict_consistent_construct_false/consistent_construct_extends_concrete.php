<?hh
// No errors because strict_consistent_construct flag is not enabled

class PlainConcrete {
  public function __construct(public int $x) {}
}
abstract class AbstractChildOfPlain extends PlainConcrete {}

<<__ConsistentConstruct>>
class ConcreteCCDirect {
  public function __construct(public int $x) {}
}
abstract class AbstractChildOfDirectCC extends ConcreteCCDirect {}

<<__ConsistentConstruct>>
abstract class AbstractCCRoot {}
class ConcreteInheritsCC extends AbstractCCRoot {}
abstract class AbstractChildOfInheritedCC extends ConcreteInheritsCC {}

class ConcreteChildOfCC extends ConcreteCCDirect {}

<<__ConsistentConstruct>>
abstract class AbstractCCParent {
  public function __construct(public int $x) {}
}
abstract class AbstractChildOfAbstractCC extends AbstractCCParent {}

class ConcreteNoCC {
  public function __construct(public int $x) {}
}
<<__ConsistentConstruct>>
abstract class AbstractCCChildOfNonCC extends ConcreteNoCC {}
