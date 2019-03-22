<?hh // partial


function superglobals_by_ref(): void {
  $_ = &$_SERVER;
}
