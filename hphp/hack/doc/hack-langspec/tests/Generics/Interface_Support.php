<?hh // strict

namespace NS_test;

function main (): void
{
  echo "------- Container -------\n\n";

  echo (Vector {1, 2} instanceof Container ?
    "V is" : "V is not") . " a Container\n";
  echo (ImmVector {1, 2} instanceof Container ?
    "IV is" : "IV is not") . " a Container\n";
  echo (Map {'x' => -1, 'a' => -4} instanceof Container ?
    "M is" : "M is **NOT**") . " a Container\n";
  echo (ImmMap {'x' => -1, 'a' => -4} instanceof Container ?
    "IM is" : "IM is **NOT**") . " a Container\n";
  echo (Set {1, 3, 5} instanceof Container ?
    "S is" : "S is **NOT**") . " a Container\n";
  echo (ImmSet {1, 3, 5} instanceof Container ?
    "IS is" : "IS is **NOT**") . " a Container\n";
  echo (Pair {1, 'abc'} instanceof Container ?
    "P is" : "P is **NOT**") . " a Container\n";
  echo (array(1,3) instanceof Container ?
    "A is" : "A is **NOT**") . " a Container\n";

  echo "\n------- KeyedContainer -------\n\n";

  echo (Vector {1, 2} instanceof KeyedContainer ?
    "V is" : "V is **NOT**") . " a KeyedContainer\n";
  echo (ImmVector {1, 2} instanceof KeyedContainer ?
    "IV is" : "IV is **NOT**") . " a KeyedContainer\n";
  echo (Map {'x' => -1, 'a' => -4} instanceof KeyedContainer ?
    "M is" : "M is **NOT**") . " a KeyedContainer\n";
  echo (ImmMap {'x' => -1, 'a' => -4} instanceof KeyedContainer ?
    "IM is" : "IM is **NOT**") . " a KeyedContainer\n";
  echo (Set {1, 3, 5} instanceof KeyedContainer ?
    "S is" : "S is **NOT**") . " a KeyedContainer\n";
  echo (ImmSet {1, 3, 5} instanceof KeyedContainer ?
    "IS is" : "IS is **NOT**") . " a KeyedContainer\n";
  echo (Pair {1, 'abc'} instanceof KeyedContainer ?
    "P is" : "P is **NOT**") . " a KeyedContainer\n";
  echo (array(1,3) instanceof KeyedContainer ?
    "A is" : "A is **NOT**") . " a KeyedContainer\n";

  echo "------- Traversable -------\n\n";

  echo (Vector {1, 2} instanceof Traversable ?
    "V is" : "V is not") . " a Traversable\n";
  echo (ImmVector {1, 2} instanceof Traversable ?
    "IV is" : "IV is not") . " a Traversable\n";
  echo (Map {'x' => -1, 'a' => -4} instanceof Traversable ?
    "M is" : "M is **NOT**") . " a Traversable\n";
  echo (ImmMap {'x' => -1, 'a' => -4} instanceof Traversable ?
    "IM is" : "IM is **NOT**") . " a Traversable\n";
  echo (Set {1, 3, 5} instanceof Traversable ?
    "S is" : "S is **NOT**") . " a Traversable\n";
  echo (ImmSet {1, 3, 5} instanceof Traversable ?
    "IS is" : "IS is **NOT**") . " a Traversable\n";
  echo (Pair {1, 'abc'} instanceof Traversable ?
    "P is" : "P is **NOT**") . " a Traversable\n";
  echo (array(1,3) instanceof Traversable ?
    "A is" : "A is **NOT**") . " a Traversable\n";

  echo "\n------- KeyedTraversable -------\n\n";

  echo (Vector {1, 2} instanceof KeyedTraversable ?
     "V is" : "V is **NOT**") . " a KeyedTraversable\n";
  echo (ImmVector {1, 2} instanceof KeyedTraversable ?
     "IV is" : "IV is **NOT**") . " a KeyedTraversable\n";
  echo (Map {'x' => -1, 'a' => -4} instanceof KeyedTraversable ?
     "M is" : "M is **NOT**") . " a KeyedTraversable\n";
  echo (ImmMap {'x' => -1, 'a' => -4} instanceof KeyedTraversable ?
     "IM is" : "IM is **NOT**") . " a KeyedTraversable\n";
  echo (Set {1, 3, 5} instanceof KeyedTraversable ?
     "S is" : "S is **NOT**") . " a KeyedTraversable\n";
  echo (ImmSet {1, 3, 5} instanceof KeyedTraversable ?
     "IS is" : "IS is **NOT**") . " a KeyedTraversable\n";
  echo (Pair {1, 'abc'} instanceof KeyedTraversable ?
     "P is" : "P is **NOT**") . " a KeyedTraversable\n";
  echo (array(1,3) instanceof KeyedTraversable ?
     "A is" : "A is **NOT**") . " a KeyedTraversable\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
