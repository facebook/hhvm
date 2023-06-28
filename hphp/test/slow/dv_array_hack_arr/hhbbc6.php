<?hh

class C {}

function marked_darray(darray $x) :mixed{
  var_dump(HH\is_php_array($x));
}

function marked_varray(varray $x) :mixed{
  var_dump(HH\is_php_array($x));
}

<<__EntryPoint>>
function main() :mixed{
  marked_darray(darray['a' => 17]);
  marked_darray(HH\array_mark_legacy(darray['a' => 17]));
  marked_darray(darray['a' => new C()]);
  marked_darray(HH\array_mark_legacy(darray['a' => new C()]));

  marked_varray(varray[17]);
  marked_varray(HH\array_mark_legacy(varray[17]));
  marked_varray(varray[new C()]);
  marked_varray(HH\array_mark_legacy(varray[new C()]));
}
