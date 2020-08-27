<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
  }

  public function bad0(mixed $data): void {
    $_ = $data is this:@T;
  }

  public function bad1<TE as this:@E>(TE $entry, mixed $data): void {
    $_ = $data is TE:@T;
  }
}

function bad2(mixed $data): void {
  $_ = $data is C:@T;
}

function bad3<TE as C:@E>(TE $entry, mixed $data): void {
  $_ = $data is TE:@T;
}
