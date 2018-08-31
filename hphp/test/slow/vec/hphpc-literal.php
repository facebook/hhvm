<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  var_dump(vec[123] == vec['123']);
  var_dump(vec[123] != vec['123']);
  var_dump(vec[123] === vec['123']);
  var_dump(vec[123] !== vec['123']);
  var_dump(vec[123] < vec['123']);
  var_dump(vec[123] <= vec['123']);
  var_dump(vec[123] > vec['123']);
  var_dump(vec[123] >= vec['123']);
  var_dump(vec[123] <=> vec['123']);

  var_dump(vec['123'] == vec[123]);
  var_dump(vec['123'] != vec[123]);
  var_dump(vec['123'] === vec[123]);
  var_dump(vec['123'] !== vec[123]);
  var_dump(vec['123'] < vec[123]);
  var_dump(vec['123'] <= vec[123]);
  var_dump(vec['123'] > vec[123]);
  var_dump(vec['123'] >= vec[123]);
  var_dump(vec['123'] <=> vec[123]);

  var_dump(vec[123] == vec[123]);
  var_dump(vec[123] != vec[123]);
  var_dump(vec[123] === vec[123]);
  var_dump(vec[123] !== vec[123]);
  var_dump(vec[123] < vec[123]);
  var_dump(vec[123] <= vec[123]);
  var_dump(vec[123] > vec[123]);
  var_dump(vec[123] >= vec[123]);
  var_dump(vec[123] <=> vec[123]);

  var_dump(vec[123] == 123);
  var_dump(vec[123] != 123);
  var_dump(vec[123] === 123);
  var_dump(vec[123] !== 123);
  try { var_dump(vec[123] < 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[123] <= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[123] > 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[123] >= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[123] <=> 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump([vec[123], vec['123']] == [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] != [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] === [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] !== [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] < [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] <= [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] > [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] >= [vec['123'], vec[123]]);
  var_dump([vec[123], vec['123']] <=> [vec['123'], vec[123]]);

  var_dump([vec[123], keyset['123']] == [keyset['123'], vec[123]]);
  var_dump([vec[123], keyset['123']] != [keyset['123'], vec[123]]);
  var_dump([vec[123], keyset['123']] === [keyset['123'], vec[123]]);
  var_dump([vec[123], keyset['123']] !== [keyset['123'], vec[123]]);
  try { var_dump([vec[123], keyset['123']] < [keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump([vec[123], keyset['123']] <= [keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump([vec[123], keyset['123']] > [keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump([vec[123], keyset['123']] >= [keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump([vec[123], keyset['123']] <=> [keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
}

<<__EntryPoint>>
function main_hphpc_literal() {
main();
}
