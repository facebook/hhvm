<?hh
function main() {
  $ctypes = array('HH\\Vector', 'Map', 'StableMap');
  foreach ($ctypes as $ctype) {
    echo "=== $ctype ===\n";
    $c = new $ctype();
    try {
      $c[0];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    try {
      $c[PHP_INT_MAX];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    try {
      $c[~PHP_INT_MAX];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    if ($ctype === 'HH\\Vector') {
      continue;
    }
    try {
      $c['abc'];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    $hundred_chars = 'abcdefghijklmnopqrst' .
                     'abcdefghijklmnopqrst' .
                     'abcdefghijklmnopqrst' .
                     'abcdefghijklmnopqrst' .
                     'abcdefghijklmnopqrst';
    try {
      $c[$hundred_chars];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    try {
      $c['ABCDEFGHIJKLMNOPQRST' . $hundred_chars];
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    try {
      $c["ABCDEFGHIJ\000KLMNOPQRST" . $hundred_chars];
    } catch (Exception $e) {
      $str = $e->getMessage();
      $len = strlen($str);
      for ($i = 0; $i < $len; ++$i) {
        if (ord($str[$i]) == 0) {
          echo '<NUL>';
        } else {
          echo($str[$i]);
        }
      }
      echo "\n";
    }
  }
}

main();

