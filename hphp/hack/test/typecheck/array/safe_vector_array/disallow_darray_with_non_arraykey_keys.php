<?hh // strict

function f<Tk, Tv>(Tk $key, Tv $value): darray<Tk, Tv> {
  return darray[$key => $value];
}
