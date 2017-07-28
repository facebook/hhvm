<?hh // strict
class Foo<
  T2,
  /* HH_FIXME[2049] */
  +T as not_real_class> {}

