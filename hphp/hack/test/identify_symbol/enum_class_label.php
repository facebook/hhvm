<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type LabelOrMember<-Tin, +Tout as arraykey> =
  | HH\EnumClass\Label<Tin, Tout>
  | \HH\MemberOf<Tin, Tout>;

abstract enum class E: arraykey {
  abstract int ID;
}

abstract class C {
  abstract const type T as E;
  public function set<T>(?LabelOrMember<this::T, T> $label, T $val): void {}
}

function main(C $cls): void {
  $cls->set(#ID, 42);
}
