<?hh

class R {
  public $dimensions;
  function __construct() {
    $this->dimensions = array('thing' => 'x');
  }
}
final class Table {
  public function insert($key, $factory) { return $key; }
}

class Other {
  public function insert($id, $data) {return false;}
}

final class Tree {
  static function go($t) {
    $things = new Table();
    $samples = function($row) use($things) {
      return array(
        'thing1_id' => $things->insert($row->dimensions['thing'], null),
        'thing2_id' => $things->insert(
          null, () ==> { return $row; })
      );
    };

    return $samples($t);
  }
}

var_dump(Tree::go(new R));
