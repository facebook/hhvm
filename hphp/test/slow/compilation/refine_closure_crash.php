<?hh

class R {
  public $dimensions;
  function __construct() {
    $this->dimensions = dict['thing' => 'x'];
  }
}
final class Table {
  public function insert($key, $factory) :mixed{ return $key; }
}

class Other {
  public function insert($id, $data) :mixed{return false;}
}

final class Tree {
  static function go($t) :mixed{
    $things = new Table();
    $samples = function($row) use($things) {
      return dict[
        'thing1_id' => $things->insert($row->dimensions['thing'], null),
        'thing2_id' => $things->insert(
          null, () ==> { return $row; })
      ];
    };

    return $samples($t);
  }
}


<<__EntryPoint>>
function main_refine_closure_crash() :mixed{
var_dump(Tree::go(new R));
}
