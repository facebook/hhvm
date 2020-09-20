<?hh //partial

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Please add the expected type parameters */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type MISSING_TYPE = UNSAFE_TYPE_HH_FIXME;
}

class C {
  public function fa(int $x): mixed {
    return 1;
  }

  public function fb(int $x): dynamic {
    return "string";
  }

  public function fc(int $x): UNSAFE_TYPE_HH_FIXME {
    return 1.0;
  }

  public function fd(int $x): HH_FIXME::MISSING_TYPE  {
    return 1;
  }
}
