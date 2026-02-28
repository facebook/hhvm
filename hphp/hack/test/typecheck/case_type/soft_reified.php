<?hh

final class SoftReified<<<__Soft>> reify T> {}

case type SoftReifiedCTDisjoint = SoftReified<int> | SoftReified<string>;

case type SoftReifiedCTNotDisjoint = SoftReified<int> | SoftReified<int>;
