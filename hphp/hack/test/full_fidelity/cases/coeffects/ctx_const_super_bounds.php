<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface PureIterator {
  abstract const ctx Tc super [];
}

interface AtMostNonDetIterator {
  abstract const ctx Tc super [non_det] = [non_det];
}

class ImpureIterator extends Iterator {
  protected const ctx Tc super [defaults] = [defaults];
}
