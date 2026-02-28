<?hh

function test_pair($k1, $v1, $k2, $v2) :mixed{
  echo "$k1 cmp $k2:\n";
  try {
    echo (($v1 === $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 !== $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 < $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 <= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 == $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 != $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 >= $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    echo (($v1 > $v2) ? "T       " : "F       ");
  } catch (Exception $e) {
    echo "Err     ";
  }
  try {
    $cmp = $v1 <=> $v2;
    echo "$cmp";
  } catch (Exception $e) {
    echo "Err";
  }

  echo "\n";
}

<<__EntryPoint>>
function main(): void {
  $aiter1 = new ArrayIterator(dict['a' => 'b']);
  $aiter2 = new ArrayIterator(dict['a' => 'b']); $aiter2->c = 'd';
  $xml1 = simplexml_load_string("<apple />");
  $xml2 = simplexml_load_string("<pie><apple /></pie>");
  $dynamicA = new stdClass(); $dynamicA->a = 'a';
  $dynamicB = new stdClass(); $dynamicB->b = 'a';
  $dynamicANAN = new stdClass(); $dynamicANAN->a = NAN;
  $dynamicAB = new stdClass(); $dynamicAB->a = 'a'; $dynamicAB->b = 'b';
  $dynamicBCThrows = new stdClass();
  $dynamicBCThrows->c = 'c';

  $pairs = vec[
    vec[
      dict['k' => 'ArrayIterator 1', 'v' => $aiter1],
      dict['k' => 'ArrayIterator 2', 'v' => $aiter2],
    ],
    vec[
      dict['k' => 'SimpleXMLElement 1', 'v' => $xml1],
      dict['k' => 'SimpleXMLElement 2', 'v' => $xml2],
    ],
    vec[
      // same number of dynamic properties with the same value, but diff name
      dict['k' => 'Dynamic property a', 'v' => $dynamicA],
      dict['k' => 'Dynamic property b', 'v' => $dynamicB],
    ],
    vec[
      dict['k' => 'Dynamic property a', 'v' => $dynamicA],
      dict['k' => 'Dynamic property NAN', 'v' => $dynamicANAN],
    ],
    vec[
      // depending on which operand in the comparison we traverse, we'll either
      // short-circuit or throw
      dict['k' => 'Dynamic props (a, b)', 'v' => $dynamicAB],
      dict['k' => 'Dynamic props (b, c)', 'v' => $dynamicBCThrows],
    ],
  ];

  echo "same    nsame   lt      lte     eq      neq     gte     gt      cmp\n\n";
  foreach ($pairs as $p) {
    test_pair($p[0]['k'], $p[0]['v'], $p[1]['k'], $p[1]['v']);
    test_pair($p[1]['k'], $p[1]['v'], $p[0]['k'], $p[0]['v']);
  }
}
