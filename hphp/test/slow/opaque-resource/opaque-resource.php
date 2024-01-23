<?hh

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  $o1 = __SystemLib\create_opaque_value(42, vec[1, 2, 3]);
  $o2 = __SystemLib\create_opaque_value(1337, vec[1, 2, 3]);

  $o3 = __SystemLib\create_opaque_value(42, vec[1, 2, 3]);
  $o4 = __SystemLib\create_opaque_value(1337, vec[1, 2, 3]);

  $o5 = __SystemLib\create_opaque_value(42, vec[1, 3]);
  $o6 = __SystemLib\create_opaque_value(1337, vec[2, 3]);

  var_dump($o1 === $o2); // F
  var_dump($o1 === $o3); // T
  var_dump($o1 === $o4); // F
  var_dump($o1 === $o5); // F
  var_dump($o1 === $o6); // F

  var_dump($o2 === $o3); // F
  var_dump($o2 === $o4); // T
  var_dump($o2 === $o5); // F
  var_dump($o2 === $o6); // F

  var_dump($o3 === $o4); // F
  var_dump($o3 === $o5); // F
  var_dump($o3 === $o6); // F

  var_dump($o4 === $o5); // F
  var_dump($o4 === $o6); // F


  var_dump($o1 == $o2); // F
  var_dump($o1 == $o3); // T
  var_dump($o1 == $o4); // F
  var_dump($o1 == $o5); // F
  var_dump($o1 == $o6); // F

  var_dump($o2 == $o3); // F
  var_dump($o2 == $o4); // T
  var_dump($o2 == $o5); // F
  var_dump($o2 == $o6); // F

  var_dump($o3 == $o4); // F
  var_dump($o3 == $o5); // F
  var_dump($o3 == $o6); // F

  var_dump($o4 == $o5); // F
  var_dump($o4 == $o6); // F

  var_dump($o1 !== $o2); // T
  var_dump($o1 != $o3);  // F

  try { var_dump($o1 <  $o3); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($o1 <= $o3); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($o1 >  $o3); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($o1 >= $o3); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { var_dump(new stdClass() <  $o3);   } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump(HH\stdout()    <= $o3);   } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($o1            >  vec[]); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { var_dump($o1            >= "123"); } catch (Exception $e) { var_dump($e->getMessage()); }

  try { __SystemLib\unwrap_opaque_value(40, $o1); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { __SystemLib\unwrap_opaque_value(42, $o2); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { __SystemLib\unwrap_opaque_value(1337, $o3); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { __SystemLib\unwrap_opaque_value(1335, $o4); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { __SystemLib\unwrap_opaque_value(0, HH\stdout()); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { __SystemLib\unwrap_opaque_value(0, new stdClass()); } catch (Exception $e) { var_dump($e->getMessage()); }

  var_dump(__SystemLib\unwrap_opaque_value(42, $o1));
  var_dump(__SystemLib\unwrap_opaque_value(1337, $o2));

  var_dump(__SystemLib\unwrap_opaque_value(42, $o3));
  var_dump(__SystemLib\unwrap_opaque_value(1337, $o4));

  var_dump(__SystemLib\unwrap_opaque_value(42, $o5));
  var_dump(__SystemLib\unwrap_opaque_value(1337, $o6));
}
