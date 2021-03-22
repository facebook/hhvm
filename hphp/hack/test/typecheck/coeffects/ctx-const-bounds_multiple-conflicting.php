<?hh

abstract class MultipleConflictingNotAllowed {
  abstract const ctx CUU as [] as [defaults];
  abstract const ctx CLL super [defaults] super [];

  abstract const ctx CULU as [] super [defaults] as [rx];
  abstract const ctx CLUL super [] as [rx] super [defaults];
}
