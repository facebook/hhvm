<?hh
interface Iterator {
  abstract const ctx Tc;
}

interface NonDetIterator extends Iterator {
  abstract const ctx Tc = [non_det];
}

interface ImpureIterator extends Iterator {
  abstract const ctx Tc = [defaults];
}

class ImpureIteratorImpl extends ImpureIterator {
  const ctx Tc = [non_det, rx];
}
