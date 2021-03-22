<?hh
interface PureIterator {
  abstract const ctx Tc super [];
}

interface AtMostNonDetIterator {
  abstract const ctx Tc as [non_det] = [non_det];
}

class ImpureIterator extends Iterator {
  protected const ctx Tc super [defaults] = [defaults];
}
