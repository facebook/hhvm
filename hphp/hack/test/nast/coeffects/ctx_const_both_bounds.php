<?hh

interface WithLowerThenUpperBound {
  const ctx C super [defaults] as [rx];
}

interface WithUpperThenLowerBound {
  const ctx C as [write_props] super [defaults];
}
