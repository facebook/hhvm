<?php

class :node {
  private $children;
  public function __construct($_, $children) {
    $this->children = $children;
  }
  public function __toString() {
    $o = '';
    foreach ($this->children as $child) {
      $o .= (string)$child;
    }
    $o .= count($this->children);
    return $o;
  }
}

$n1 =
  <node>
    <!--hey-->
  </node>;

$n2 =
  <node>
    {'a'}
    <!--hey-->
    <!--
      multiline comment
      with multiple lines
    -->
    {'b'}
    <node>
      {'c'}
      <!--nested comment-->
    </node>
    <!--comment-with -- extra--hyphens----->
  </node>;

echo
  (string)$n1.
  (string)$n2.
  "\n";
