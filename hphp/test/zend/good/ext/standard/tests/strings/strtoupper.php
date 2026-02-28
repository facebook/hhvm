<?hh
<<__EntryPoint>>
function main_entry(): void {
  $chars = "\xe4\xf6\xfc";
  // Not sure which is most portable. BSD's answer to
  // this one. A small array based on PHP_OS should
  // cover a majority of systems and makes the problem
  // of locales transparent for the end user.
  setlocale(LC_CTYPE, "de_DE", "de", "german", "ge", "de_DE.ISO8859-1", "ISO8859-1");
  echo strtoupper($chars)."\n";
}
