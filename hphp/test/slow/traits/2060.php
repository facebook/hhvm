<?hh
function must_prepare($x) :mixed{
  if ($x == MUST_PREPARE) {
     $x = 2;
  }
  return $x;
}
trait EntWithViewerComments {
  public $entCanComment = 1;
  public function canViewerComment() :mixed{
    var_dump($this->entCanComment);
    var_dump(must_prepare($this->entCanComment));
  }
}
class EntShare {
  use EntWithViewerComments;
}


const MUST_PREPARE = 0;
<<__EntryPoint>>
function main_2060() :mixed{
$obj1 = new EntShare;
$obj1->canViewerComment();
}
