<?hh

function main() :mixed{
  $ctypes = dict[
    'Vector' => new Vector(),
    'Map' => new Map(),
  ];
  foreach ($ctypes as $ctype => $c) {
    echo "=== $ctype ===\n";
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
    if ($ctype === 'Vector') {
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


<<__EntryPoint>>
function main_817() :mixed{
main();
}
