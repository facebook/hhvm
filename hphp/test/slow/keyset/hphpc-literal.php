<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() :mixed{
  var_dump(keyset[123] == keyset['123']);
  var_dump(keyset[123] != keyset['123']);
  var_dump(keyset[123] === keyset['123']);
  var_dump(keyset[123] !== keyset['123']);
  try { var_dump(keyset[123] < keyset['123']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <= keyset['123']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] > keyset['123']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] >= keyset['123']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <=> keyset['123']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(keyset['123'] == keyset[123]);
  var_dump(keyset['123'] != keyset[123]);
  var_dump(keyset['123'] === keyset[123]);
  var_dump(keyset['123'] !== keyset[123]);
  try { var_dump(keyset['123'] < keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset['123'] <= keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset['123'] > keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset['123'] >= keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset['123'] <=> keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(keyset[123] == keyset[123]);
  var_dump(keyset[123] != keyset[123]);
  var_dump(keyset[123] === keyset[123]);
  var_dump(keyset[123] !== keyset[123]);
  try { var_dump(keyset[123] < keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <= keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] > keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] >= keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <=> keyset[123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(keyset[123] == 123);
  var_dump(keyset[123] != 123);
  var_dump(keyset[123] === 123);
  var_dump(keyset[123] !== 123);
  try { var_dump(keyset[123] < 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] > 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] >= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[123] <=> 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  try { var_dump(keyset[false] === keyset[true]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[1.234] === keyset[5.678]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(keyset[null] === keyset[null]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(vec[keyset[123], keyset['123']] == vec[keyset['123'], keyset[123]]);
  var_dump(vec[keyset[123], keyset['123']] != vec[keyset['123'], keyset[123]]);
  var_dump(vec[keyset[123], keyset['123']] === vec[keyset['123'], keyset[123]]);
  var_dump(vec[keyset[123], keyset['123']] !== vec[keyset['123'], keyset[123]]);
  try { var_dump(vec[keyset[123], keyset['123']] < vec[keyset['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], keyset['123']] <= vec[keyset['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], keyset['123']] > vec[keyset['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], keyset['123']] >= vec[keyset['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], keyset['123']] <=> vec[keyset['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(vec[keyset[123], vec['123']] == vec[vec['123'], keyset[123]]);
  var_dump(vec[keyset[123], vec['123']] != vec[vec['123'], keyset[123]]);
  var_dump(vec[keyset[123], vec['123']] === vec[vec['123'], keyset[123]]);
  var_dump(vec[keyset[123], vec['123']] !== vec[vec['123'], keyset[123]]);
  try { var_dump(vec[keyset[123], vec['123']] < vec[vec['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], vec['123']] <= vec[vec['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], vec['123']] > vec[vec['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], vec['123']] >= vec[vec['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[keyset[123], vec['123']] <=> vec[vec['123'], keyset[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
}

<<__EntryPoint>>
function main_hphpc_literal() :mixed{
main();
}
