<?hh

function f<Tk, Tv>(Tk $key, Tv $value): darray<Tk, Tv> {
  return dict[$key => $value];
}
