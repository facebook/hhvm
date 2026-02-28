<?hh

function cast_varray<Tv>(varray<Tv> $x): nonnull {
  return $x;
}

function cast_darray<Tk, Tv>(darray<Tk, Tv> $x): nonnull {
  return $x;
}

function cast_array1<Tv>(varray<Tv> $x): nonnull {
  return $x;
}

function cast_array2<Tk, Tv>(darray<Tk, Tv> $x): nonnull {
  return $x;
}

function cast_varray_or_darray<Tk>(varray_or_darray<Tk> $x): nonnull {
  return $x;
}

function empty_array(): nonnull {
  return vec[];
}

function array_used_like_a_shape(): nonnull {
  return dict['pi' => 3.1415926, 'e' => 2.718281829459045];
}

function array_used_like_a_tuple(): nonnull {
  return vec[42, "meaning of life"];
}
