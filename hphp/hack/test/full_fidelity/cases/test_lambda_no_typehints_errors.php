<?hh

/* There should be no errors due to the absence of typehints on $a and $b */
function test():void {
  $lambda = ($a, $b) ==> {
    return 1;
  };

  bar(($a, $b) ==> {
    return 1;
  });
}
