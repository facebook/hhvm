<?hh // strict
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class C {
  enum E {
    case type T;
  }

  public function bad0(mixed $data): void {
    $_ = $data as this:@T;
  }

  public function bad1<TE as this:@E>(TE $entry, mixed $data): void {
    $_ = $data as TE:@T;
  }
}

function bad2(mixed $data): void {
  $_ = $data as C:@T;
}

function bad3<TE as C:@E>(TE $entry, mixed $data): void {
  $_ = $data as TE:@T;
}
