<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function report($e) {
  echo "Exception: {$e->getMessage()}\n";
}

function test_all() {
  $x = vec[
    varray[],
    darray[],
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]),
    varray[1, 2, 3],
    darray[0 => 1, 1 => 2, 2 => 3],
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3])
  ];
  $x = __hhvm_intrinsics\launder_value($x);

  foreach ($x as $v1) {
    foreach ($x as $v2) {
      try { $v1 == $v2; }  catch (Exception $e) { report($e); }
      try { $v1 != $v2; }  catch (Exception $e) { report($e); }
      try { $v1 === $v2; } catch (Exception $e) { report($e); }
      try { $v1 !== $v2; } catch (Exception $e) { report($e); }
      try { $v1 < $v2; }   catch (Exception $e) { report($e); }
      try { $v1 > $v2; }   catch (Exception $e) { report($e); }
      try { $v1 <=> $v2; } catch (Exception $e) { report($e); }
    }
  }
}

function test_literals() {
  try { varray[] == darray[]; } catch (Exception $e) { report($e); }
  try { varray[] == __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] == varray[]; } catch (Exception $e) { report($e); }
  try { darray[] == __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) == varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) == darray[]; } catch (Exception $e) { report($e); }
  try { darray[] == darray[]; } catch (Exception $e) { report($e); }

  try { varray[] != darray[]; } catch (Exception $e) { report($e); }
  try { varray[] != __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] != varray[]; } catch (Exception $e) { report($e); }
  try { darray[] != __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) != varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) != darray[]; } catch (Exception $e) { report($e); }
  try { darray[] != darray[]; } catch (Exception $e) { report($e); }

  try { varray[] === darray[]; } catch (Exception $e) { report($e); }
  try { varray[] === __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] === varray[]; } catch (Exception $e) { report($e); }
  try { darray[] === __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) === varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) === darray[]; } catch (Exception $e) { report($e); }
  try { darray[] === darray[]; } catch (Exception $e) { report($e); }

  try { varray[] !== darray[]; } catch (Exception $e) { report($e); }
  try { varray[] !== __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] !== varray[]; } catch (Exception $e) { report($e); }
  try { darray[] !== __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) !== varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) !== darray[]; } catch (Exception $e) { report($e); }
  try { darray[] !== darray[]; } catch (Exception $e) { report($e); }

  try { varray[] < darray[]; } catch (Exception $e) { report($e); }
  try { varray[] < __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] < varray[]; } catch (Exception $e) { report($e); }
  try { darray[] < __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) < varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) < darray[]; } catch (Exception $e) { report($e); }
  try { darray[] < darray[]; } catch (Exception $e) { report($e); }

  try { varray[] > darray[]; } catch (Exception $e) { report($e); }
  try { varray[] > __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] > varray[]; } catch (Exception $e) { report($e); }
  try { darray[] > __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) > varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) > darray[]; } catch (Exception $e) { report($e); }
  try { darray[] > darray[]; } catch (Exception $e) { report($e); }

  try { varray[] <=> darray[]; } catch (Exception $e) { report($e); }
  try { varray[] <=> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { darray[] <=> varray[]; } catch (Exception $e) { report($e); }
  try { darray[] <=> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]); } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) <=> varray[]; } catch (Exception $e) { report($e); }
  try { __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]) <=> darray[]; } catch (Exception $e) { report($e); }
  try { darray[] <=> darray[]; } catch (Exception $e) { report($e); }
}

function handler($errno, $errstr) { throw new Exception($errstr); }


<<__EntryPoint>>
function main_dv_comparisons() {
echo "============== test_all ==============================\n";
test_all();
echo "============== literals ==============================\n";
test_literals();

set_error_handler(fun('handler'));

echo "============== test_all (exn) ========================\n";
test_all();
echo "============== literals (exn) ========================\n";
test_literals();
}
