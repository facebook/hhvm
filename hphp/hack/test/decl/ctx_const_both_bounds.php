<?hh

interface WithLowerThenUpperBound {
  abstract const ctx C super [defaults] as [rx];
}

interface WithUpperThenLowerBound {
  abstract const ctx C as [write_props] super [defaults];
}
