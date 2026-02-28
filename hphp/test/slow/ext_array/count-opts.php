<?hh

class Counted implements Countable {
  public function count() :mixed{ return 5; }
}
class NotCounted {
  public function count() :mixed{ return 4; }
}
class BadCounted implements Countable {
  public function count() :mixed{ throw new Exception("y u do dis"); }
}

function res() :mixed{ return count(HH\stdin()); }
function staticArr() :mixed{ return count(vec[3,2,1,0]); }
function packed() :mixed{ return count(vec[3,2,1,new stdClass]); }
function vect() :mixed{ return count(HH\Vector{5,7,8,3}); }
function counted_obj() :mixed{ return count(new Counted); }
function not_counted_obj() :mixed{ return count(new NotCounted); }
function bad_counted_obj() :mixed{ return count(new BadCounted); }

function append($arr) :mixed{
  var_dump(count($arr));

  $new_arr = $arr;
  $new_arr[] = 1;

  var_dump(count($new_arr));
}


<<__EntryPoint>>
function main_count_opts() :mixed{
append(vec[3,2]);
append(HH\Vector{3,2});

var_dump(res());
var_dump(vect());
var_dump(packed());
var_dump(staticArr());
var_dump(counted_obj());
var_dump(not_counted_obj());

try {
  var_dump(bad_counted_obj());
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
