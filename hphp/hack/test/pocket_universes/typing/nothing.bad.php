<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
    case T value;
  }
}

function good<TP as C:@E>(TP $x): TP:@T {
  return C:@E::value($x);
}

function good2<TP as C:@E, TR>(TP $x): TR where TR = TP:@T {
  return C:@E::value($x);
}

function bad<TP as C:@E>(TP $x): nothing {
  return C:@E::value($x);
}

function bad2<TP as C:@E, TR>(TP $x): TR where TR = nothing {
  return C:@E::value($x);
}

function bad3(C:@E $x): nothing {
  return C:@E::value($x);
}
