<?hh
/**
 * keys of an array that results from array_filter on a Container are mixed
 */
function test(Container<?X> $x) {
  take_int(key_type(array_filter($x)));
}

function take_int(int $x): void {}

function key_type<Tk, Tv>(array<Tk, Tv> $x): Tk {
  //UNSAFE
}
