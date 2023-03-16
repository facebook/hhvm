<?hh

function takes_arraykey(arraykey $_) : void {}

function redundant_cast(arraykey $a, int $i): void {
  takes_arraykey(HH\FIXME\UNSAFE_CAST<mixed, arraykey>($a));
  takes_arraykey(HH\FIXME\UNSAFE_CAST<mixed, arraykey>($i));
}
