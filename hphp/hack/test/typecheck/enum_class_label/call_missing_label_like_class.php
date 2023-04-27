<?hh

enum class E: mixed {
  string very_classy = "top hat";
}

function takes_label(HH\EnumClass\Label<E, mixed> $_): void {}

function call_it(): void {
  // We want #very_classy, not #class.
  takes_label(#classy);
}
