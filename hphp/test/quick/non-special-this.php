<?hh

class Glub {
  public function twee() {
    // $zzz should be the only local with an id
    $zzz = $this->twoo;
    return $zzz;
  }
}

function gurgle($g) {
  switch ($g->twee()) {
    case 1:
      echo "wtf??\n";
      break;
  }
  $arr = array('this' => 43);
  extract(&$arr);
  var_dump($this);
}

$g = new Glub();
$g->twoo = 2;
gurgle($g);
