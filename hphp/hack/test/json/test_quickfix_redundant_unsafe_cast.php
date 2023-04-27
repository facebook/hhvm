<?hh


function takes_arraykey(arraykey $_) : void {}

function redundant_cast(int $x): void {
  takes_arraykey(HH\FIXME\UNSAFE_CAST<mixed, arraykey>($x));
}

function useful_but_loose_cast(?int $x): void {
  takes_arraykey(HH\FIXME\UNSAFE_CAST<mixed,arraykey>($x));
}

function useful_and_accurate_cast(?int $x): void {
  takes_arraykey(HH\FIXME\UNSAFE_CAST<?int,arraykey>($x));
}
