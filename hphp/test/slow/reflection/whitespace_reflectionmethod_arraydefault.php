<?hh

class Herp {
  function derp($derp = varray[1,2,3]) :mixed{}
}


<<__EntryPoint>>
function main_whitespace_reflectionmethod_arraydefault() :mixed{
var_dump((string) (new ReflectionClass('Herp')));
}
