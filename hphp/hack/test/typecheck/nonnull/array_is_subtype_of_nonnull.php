<?hh // partial

function cast_array(array $x): nonnull {
  return $x;
}

function cast_varray<Tv>(varray<Tv> $x): nonnull {
  return $x;
}

function cast_darray<Tk, Tv>(darray<Tk, Tv> $x): nonnull {
  return $x;
}

function cast_array1<Tv>(array<Tv> $x): nonnull {
  return $x;
}

function cast_array2<Tk, Tv>(array<Tk, Tv> $x): nonnull {
  return $x;
}

function cast_varray_or_darray<Tk>(varray_or_darray<Tk> $x): nonnull {
  return $x;
}

function empty_array(): nonnull {
  return varray[];
}

function array_used_like_a_shape(): nonnull {
  return darray['pi' => 3.1415926, 'e' => 2.718281829459045];
}

function array_used_like_a_tuple(): nonnull {
  return varray[42, "meaning of life"];
}
