<?hh // strict

function takes_rx_traverable<T>(HH\Rx\Traversable<T> $x): HH\Rx\Traversable<T> {
  return $x;
}

function takes_rx_keyedtraverable<Tk, Tv>(
  HH\Rx\KeyedTraversable<Tk,Tv> $x
): HH\Rx\KeyedTraversable<Tk,Tv> {
  return $x;
}


<<__EntryPoint>>
function main_arrays_as_rx_traversable() {
takes_rx_traverable(vec[]);
takes_rx_traverable(dict[]);
takes_rx_traverable(keyset[]);
takes_rx_traverable(varray[]);
takes_rx_traverable(darray[]);
takes_rx_traverable(varray[]);
takes_rx_keyedtraverable(vec[]);
takes_rx_keyedtraverable(dict[]);
takes_rx_keyedtraverable(keyset[]);
takes_rx_keyedtraverable(varray[]);
takes_rx_keyedtraverable(darray[]);
takes_rx_keyedtraverable(varray[]);
}
