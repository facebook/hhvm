<?hh

function x($y, $k) :mixed{
  var_dump($k($y));
  var_dump(floor($y));
}


<<__EntryPoint>>
function main_floor_ceil() :mixed{
x(varray[1,2,3,4], 'floor');
x(varray[1,2,3,4], 'ceil');
}
