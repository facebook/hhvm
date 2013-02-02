<?php
// Extremely simple implementation of XHP client library useful primarily for
// unit tests.
class :x {
  public function __construct($attrs, $children) {
    $this->attrs = $attrs;
    $this->children = $children;
  }

  public function __toString() {
    $head = '<x';
    foreach ($this->attrs as $key => $val) {
      $head .= htmlspecialchars($key).'='.htmlspecialchars($val);
    }
    return $head.'>'.implode('', $this->children).'</x>';
  }
}
