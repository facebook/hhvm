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
  marked_darray(dict['a' => 17]);
  marked_darray(HH\array_mark_legacy(dict['a' => 17]));
  marked_darray(dict['a' => new C()]);
  marked_darray(HH\array_mark_legacy(dict['a' => new C()]));

  marked_varray(vec[17]);
  marked_varray(HH\array_mark_legacy(vec[17]));
  marked_varray(vec[new C()]);
  marked_varray(HH\array_mark_legacy(vec[new C()]));
}
