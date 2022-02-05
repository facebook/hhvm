<?hh

class MyPolicy {}
class MySubPolicy extends MyPolicy {}

interface Base {
  // NOTE: without `super ...`, `m` would be uncallable
  // unless `zoned_with` was part of [defaults] (which is wrong)
  abstract const ctx C super [zoned_with<\MyPolicy>];
  public function m()[this::C]: void;
}

function good_caller1(Base $b)[zoned_with<\MyPolicy>]: void {
  $b->m(); // OK: $b::C >: policied_of<MyPolicy>
}

function bad_caller1(Base $b)[zoned]: void {
  $b->m(); // ERROR: zoned <:/ zoned_with<...>
}

function bad_caller2(Base $b)[zoned_with<\MySubPolicy>]: void {
  $b->m(); // ERROR: zoned_with<MySubPolicy> <: zoned_with<MyPolicy>
}          //       (zoned_with is invariant)
