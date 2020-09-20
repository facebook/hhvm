<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T as string;
    :@A(
      type T = string
    );
  }
}

function test0(C:@E $x): string {
  return $x;
}

function test1<TP as C:@E>(TP $x): string {
  return $x;
}

function test2<TP as C:@E>(TP:@T $x): string {
  return $x;
}

function test3<TP as C:@E>(TP $y, TP:@T $x): string {
  return $x;
}
