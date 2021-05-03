<?hh

abstract class MultipleConflictingNotAllowed {
  abstract const ctx CUU as [] as [defaults];
  abstract const ctx CLL super [defaults] super [];

  abstract const ctx CULU as [] super [defaults] as [read_globals];
  abstract const ctx CLUL super [] as [read_globals] super [defaults];
}
