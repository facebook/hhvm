<?hh // strict

function test<Tk, Tv>(Tk $key, Tv $value): void {
  $_ = Map { $key => $value };
  $_ = Set { $key };
  $_ = dict[$key => $value];
  $_ = keyset[$key];
  $_ = darray[$key => $value];
}
