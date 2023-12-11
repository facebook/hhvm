<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() :mixed{
  var_dump(dict[123 => 'abc'] == dict['123' => 'abc']);
  var_dump(dict[123 => 'abc'] != dict['123' => 'abc']);
  var_dump(dict[123 => 'abc'] === dict['123' => 'abc']);
  var_dump(dict[123 => 'abc'] !== dict['123' => 'abc']);
  try { var_dump(dict[123 => 'abc'] < dict['123' => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <= dict['123' => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] > dict['123' => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] >= dict['123' => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <=> dict['123' => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(dict['123' => 'abc'] == dict[123 => 'abc']);
  var_dump(dict['123' => 'abc'] != dict[123 => 'abc']);
  var_dump(dict['123' => 'abc'] === dict[123 => 'abc']);
  var_dump(dict['123' => 'abc'] !== dict[123 => 'abc']);
  try { var_dump(dict['123' => 'abc'] < dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict['123' => 'abc'] <= dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict['123' => 'abc'] > dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict['123' => 'abc'] >= dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict['123' => 'abc'] <=> dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(dict[123 => 'abc'] == dict[123 => 'abc']);
  var_dump(dict[123 => 'abc'] != dict[123 => 'abc']);
  var_dump(dict[123 => 'abc'] === dict[123 => 'abc']);
  var_dump(dict[123 => 'abc'] !== dict[123 => 'abc']);
  try { var_dump(dict[123 => 'abc'] < dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <= dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] > dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] >= dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <=> dict[123 => 'abc']); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(dict[123 => 'abc'] == 123);
  var_dump(dict[123 => 'abc'] != 123);
  var_dump(dict[123 => 'abc'] === 123);
  var_dump(dict[123 => 'abc'] !== 123);
  try { var_dump(dict[123 => 'abc'] < 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] > 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] >= 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[123 => 'abc'] <=> 123); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  try { var_dump(dict[false => 123] === dict[true => 123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[1.234 => 123] === dict[5.678 => 123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(dict[null => 123] === dict[null => 123]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(HH\Lib\Legacy_FIXME\eq(vec[dict[0 => 123], dict[0 => '123']], vec[dict[0 => '123'], dict[0 => 123]]));
  var_dump(HH\Lib\Legacy_FIXME\neq(vec[dict[0 => 123], dict[0 => '123']], vec[dict[0 => '123'], dict[0 => 123]]));
  var_dump(vec[dict[0 => 123], dict[0 => '123']] === vec[dict[0 => '123'], dict[0 => 123]]);
  var_dump(vec[dict[0 => 123], dict[0 => '123']] !== vec[dict[0 => '123'], dict[0 => 123]]);
  try { var_dump(vec[dict[0 => 123], dict[0 => '123']] < vec[dict[0 => '123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], dict[0 => '123']] <= vec[dict[0 => '123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], dict[0 => '123']] > vec[dict[0 => '123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], dict[0 => '123']] >= vec[dict[0 => '123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], dict[0 => '123']] <=> vec[dict[0 => '123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }

  var_dump(vec[dict[0 => 123], vec['123']] == vec[vec['123'], dict[0 => 123]]);
  var_dump(vec[dict[0 => 123], vec['123']] != vec[vec['123'], dict[0 => 123]]);
  var_dump(vec[dict[0 => 123], vec['123']] === vec[vec['123'], dict[0 => 123]]);
  var_dump(vec[dict[0 => 123], vec['123']] !== vec[vec['123'], dict[0 => 123]]);
  try { var_dump(vec[dict[0 => 123], vec['123']] < vec[vec['123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], vec['123']] <= vec[vec['123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], vec['123']] > vec[vec['123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], vec['123']] >= vec[vec['123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
  try { var_dump(vec[dict[0 => 123], vec['123']] <=> vec[vec['123'], dict[0 => 123]]); } catch (Exception $e) { echo $e->getMessage() . "\n"; }
}

<<__EntryPoint>>
function main_hphpc_literal() :mixed{
main();
}
