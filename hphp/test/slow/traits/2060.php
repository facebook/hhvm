<?php

define('MUST_PREPARE', 0);
function must_prepare($x) {
  if ($x == MUST_PREPARE) {
     $x = 2;
  }
  return $x;
}
trait EntWithViewerComments {
  public $entCanComment = 1;
  public function canViewerComment() {
    var_dump($this->entCanComment);
    var_dump(must_prepare($this->entCanComment));
  }
}
class EntShare {
  use EntWithViewerComments;
}
$obj1 = new EntShare;
$obj1->canViewerComment();
