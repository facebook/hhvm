<?hh

function takes_int_tail(shape(int...) $_): void {}

function takes_supportdyn_int_tail(supportdyn<shape(int...)> $_): void {}

function plain_to_supportdyn_bad(shape(string...) $strings): void {
  takes_supportdyn_int_tail($strings);
}

function supportdyn_to_plain_bad(
  supportdyn<shape(string...)> $strings,
): void {
  takes_int_tail($strings);
}

function supportdyn_to_supportdyn_bad(
  supportdyn<shape(string...)> $strings,
): void {
  takes_supportdyn_int_tail($strings);
}

function supportdyn_good(supportdyn<shape(int...)> $ints): void {
  takes_int_tail($ints);
  takes_supportdyn_int_tail($ints);
}

function plain_good(shape(int...) $ints): void {
  takes_supportdyn_int_tail($ints);
}
