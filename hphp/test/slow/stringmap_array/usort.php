<?hh

function cow_usort($arr) {
  usort($arr,
        function($x, $y) {
          if ($x === $y) {
            return 0;
          } else if ($x < $y) {
            return -1;
          } else {
            return 1;
          }
        });
}

function main() {
  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;

  usort($a,
        function ($x, $y) {
          if ($x === $y) {
            return 0;
          } else if ($x < $y) {
            return -1;
          } else {
            return 1;
          }
        });
  $a[] = 'no warning';

  $a = msarray();
  $a['foo'] = 1;
  $a['bar'] = 2;
  cow_usort($a);
  $a[] = "warning";
}

main();
