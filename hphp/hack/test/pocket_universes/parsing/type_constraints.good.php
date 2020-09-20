<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class U {}
class D {}
class E {}

class C {
  enum E {
    case type T0;
    case type reify T1;
    case type T2 as U;
    case type reify T3 as U;
    case type T4 super D;
    case type reify T5 super D;
    case type T6 as E super E;
    case type reify T7 as E super E;
  }
}
