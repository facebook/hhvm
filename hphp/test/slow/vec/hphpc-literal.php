<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() :mixed{
  var_dump(HH\Lib\Legacy_FIXME\eq(vec[123], vec['123']));
  var_dump(HH\Lib\Legacy_FIXME\neq(vec[123], vec['123']));
  var_dump(vec[123] === vec['123']);
  var_dump(vec[123] !== vec['123']);
  var_dump(HH\Lib\Legacy_FIXME\lt(vec[123], vec['123']));
  var_dump(HH\Lib\Legacy_FIXME\lte(vec[123], vec['123']));
  var_dump(HH\Lib\Legacy_FIXME\gt(vec[123], vec['123']));
  var_dump(HH\Lib\Legacy_FIXME\gte(vec[123], vec['123']));
  var_dump(HH\Lib\Legacy_FIXME\cmp(vec[123], vec['123']));

  var_dump(HH\Lib\Legacy_FIXME\eq(vec['123'], vec[123]));
  var_dump(HH\Lib\Legacy_FIXME\neq(vec['123'], vec[123]));
  var_dump(vec['123'] === vec[123]);
  var_dump(vec['123'] !== vec[123]);
  var_dump(HH\Lib\Legacy_FIXME\lt(vec['123'], vec[123]));
  var_dump(HH\Lib\Legacy_FIXME\lte(vec['123'], vec[123]));
  var_dump(HH\Lib\Legacy_FIXME\gt(vec['123'], vec[123]));
  var_dump(HH\Lib\Legacy_FIXME\gte(vec['123'], vec[123]));
  var_dump(HH\Lib\Legacy_FIXME\cmp(vec['123'], vec[123]));

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

  var_dump(HH\Lib\Legacy_FIXME\eq(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(HH\Lib\Legacy_FIXME\neq(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(vec[vec[123], vec['123']] === vec[vec['123'], vec[123]]);
  var_dump(vec[vec[123], vec['123']] !== vec[vec['123'], vec[123]]);
  var_dump(HH\Lib\Legacy_FIXME\lt(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(HH\Lib\Legacy_FIXME\lte(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(HH\Lib\Legacy_FIXME\gt(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(HH\Lib\Legacy_FIXME\gte(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));
  var_dump(HH\Lib\Legacy_FIXME\cmp(vec[vec[123], vec['123']], vec[vec['123'], vec[123]]));

  var_dump(vec[vec[123], keyset['123']] == vec[keyset['123'], vec[123]]);
  var_dump(vec[vec[123], keyset['123']] != vec[keyset['123'], vec[123]]);
  var_dump(vec[vec[123], keyset['123']] === vec[keyset['123'], vec[123]]);
  var_dump(vec[vec[123], keyset['123']] !== vec[keyset['123'], vec[123]]);
  try { var_dump(vec[vec[123], keyset['123']] < vec[keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[vec[123], keyset['123']] <= vec[keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[vec[123], keyset['123']] > vec[keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[vec[123], keyset['123']] >= vec[keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[vec[123], keyset['123']] <=> vec[keyset['123'], vec[123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
}

<<__EntryPoint>>
function main_hphpc_literal() :mixed{
main();
}
