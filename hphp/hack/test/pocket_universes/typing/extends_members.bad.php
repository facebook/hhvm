<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
    case T value;
    :@I(
      type T = int,
      value = 42
    );
    :@S(
      type T = string,
      value = "foo"
    );
  }
}

class D extends C {
  enum E {
    :@X(
      type T = int,
      value = 1664
    );
  }
}

function main(): void {
  foreach (D:@E::Members() as $member) {
    $x = C:@E::value($member);
    echo $x;
  }
}
