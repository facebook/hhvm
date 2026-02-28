<?hh

// Used to represent resources
class __resource {
  public function __toString() {
    return hphp_to_string($this);
  }
}
