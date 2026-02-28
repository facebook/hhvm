<?hh


function level0(int $x0): void {
  // good

  $level1 = (int $x1) ==> {
   // good

    $level2 = (int $x2) ==> {
    // bad

      $level3 = (int $x3) ==> {
      // good
        return "";
      };
      if ($x2 == 1) {
        return "";
      } else {
        return;
      }
    };
    echo "";
  };
}
