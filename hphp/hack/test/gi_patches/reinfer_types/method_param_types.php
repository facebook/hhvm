<?hh //partial

type UNSAFE_TYPE_HH_FIXME_<T> = T;

/* HH_FIXME[4101] Please add the expected type parameters */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

abstract final class HH_FIXME {
  const type MISSING_TYPE = UNSAFE_TYPE_HH_FIXME;
}

class C {
  public function fa(int $_,  mixed  $x): string {
    /* HH_FIXME[4110] Invalid return type */
    return $x;
  }

  public function fb(dynamic $x): int {
    return $x;
  }

  public function fc(UNSAFE_TYPE_HH_FIXME $x): int {
    return $x;
  }

  public function fd(HH_FIXME::MISSING_TYPE $x): float {
    return $x;
  }
}
