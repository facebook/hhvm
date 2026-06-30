<?hh

// Used to represent resources
class __resource {
  public function __toString(): string {
    /* HH_FIXME[4110] hphp_to_string() now `?string` */
    return hphp_to_string($this);
  }
}
