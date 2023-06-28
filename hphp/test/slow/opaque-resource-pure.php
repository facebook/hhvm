<?hh

function fun_with_write_props()[]: void {
  $o1 = __SystemLib\create_opaque_value(42, vec[1, 2, 3]);
  $o2 = __SystemLib\create_opaque_value(1337, vec[1, 2, 3]);

  $o3 = __SystemLib\create_opaque_value(42, vec[1, 2, 3]);
  $o4 = __SystemLib\create_opaque_value(1337, vec[1, 2, 3]);

  $o5 = __SystemLib\create_opaque_value(42, vec[1, 3]);
  $o6 = __SystemLib\create_opaque_value(1337, vec[2, 3]);

  __SystemLib\unwrap_opaque_value(42, $o1);
  __SystemLib\unwrap_opaque_value(1337, $o2);

  __SystemLib\unwrap_opaque_value(42, $o3);
  __SystemLib\unwrap_opaque_value(1337, $o4);

  __SystemLib\unwrap_opaque_value(42, $o5);
  __SystemLib\unwrap_opaque_value(1337, $o6);
}

<<__EntryPoint>>
function main() :mixed{
  fun_with_write_props();
  print("Success");
}
