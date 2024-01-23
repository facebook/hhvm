<?hh

function charsAsHexSeq($s) :mixed{
  if (!is_string($s)) {
    return $s;
  }
  $out = '';
  for ($i = 0, $len = strlen($s); $i < $len; $i++) {
    $out .= '\x' . strtoupper(dechex(ord($s[$i])));
  }
  return $out;
}

class CountCharsTest {
  function __toString()[] :mixed{
    return 'hhvm';
  }
}


<<__EntryPoint>>
function main_count_chars(): mixed {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  echo "mode 0\n";
  var_dump(array_filter(count_chars('', 0)));
  var_dump(array_filter(count_chars('hhvm', 0)));
  try { var_dump(array_filter(count_chars(new CountCharsTest, 0))); } catch (Exception $e) { var_dump($e->getMessage()); }

  echo "\nmode 1\n";
  var_dump(count_chars('', 1));
  var_dump(count_chars('hhvm', 1));
  try { var_dump(count_chars(new CountCharsTest, 1)); } catch (Exception $e) { var_dump($e->getMessage()); }

  echo "\nmode 2\n";
  var_dump(count_chars('', 2));
  var_dump(count_chars('hhvm', 2));
  try { var_dump(count_chars(new CountCharsTest, 2)); } catch (Exception $e) { var_dump($e->getMessage()); }

  echo "\nmode 3\n";
  var_dump(count_chars('', 3));
  var_dump(count_chars('hhvm', 3));
  try { var_dump(count_chars(new CountCharsTest, 3)); } catch (Exception $e) { var_dump($e->getMessage()); }

  echo "\nmode 4\n";
  var_dump(charsAsHexSeq(count_chars('', 4)));
  var_dump(charsAsHexSeq(count_chars('hhvm', 4)));
  try { var_dump(charsAsHexSeq(count_chars(new CountCharsTest, 4))); } catch (Exception $e) { var_dump($e->getMessage()); }
}
