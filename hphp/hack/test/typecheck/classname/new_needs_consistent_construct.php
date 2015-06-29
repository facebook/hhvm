<?hh // strict

abstract class C {}

// needs consistent construct, since classname<ChildOfC>
// is a classname<C>
function factory(classname<C> $klass): C {
  return new $klass();
}
