<?hh //partial

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Please add the expected type parameters */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type MISSING_TYPE = UNSAFE_TYPE_HH_FIXME;
}

class X {
  public mixed $a = "string";
  private dynamic $b = 1;
  protected UNSAFE_TYPE_HH_FIXME $c = 1.0;
  protected HH_FIXME::MISSING_TYPE $d = "string";
}
