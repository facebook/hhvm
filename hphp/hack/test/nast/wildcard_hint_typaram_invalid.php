<?hh

class WildcardTyparam<T> {}

function wildcard_hint_typaram(WildcardTyparam<_> $x): void {}
