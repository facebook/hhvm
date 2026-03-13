<?hh
<<__ConsistentConstruct>>
class ConcreteCCDirect {
  public function __construct(public int $x) {}
}
// ERROR: abstract class extending a nonabstract __ConsistentConstruct class
abstract class AbstractChildOfDirectCC extends ConcreteCCDirect {}

<<__ConsistentConstruct>>
abstract class AbstractCCRoot {}
class ConcreteInheritsCC extends AbstractCCRoot {}
// ERROR: abstract class extending a nonabstract __ConsistentConstruct class
// where the parent get __ConsistentConstruct via inheritance
abstract class AbstractChildOfInheritedCC extends ConcreteInheritsCC {}
