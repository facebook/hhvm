<?hh

class MyPolicy {}
class MySubPolicy extends MyPolicy {}

interface Base {
  // NOTE: without `super ...`, `m` would be uncallable
  // unless `policied_of` was part of [defaults] (which is wrong)
  abstract const ctx C super [policied_of<\MyPolicy>];
  public function m()[this::C]: void;
}

function good_caller1(Base $b)[policied_of<\MyPolicy>]: void {
  $b->m(); // OK: $b::C >: policied_of<MyPolicy>
}

function bad_caller1(Base $b)[policied]: void {
  $b->m(); // ERROR: policied <:/ policied_of<...>
}

function bad_caller2(Base $b)[policied_of<\MySubPolicy>]: void {
  $b->m(); // ERROR: policied_of<MySubPolicy> <: policied_of<MyPolicy>
}          //       (policied_of is invariant)
