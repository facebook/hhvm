<?hh

function main(): void {
  $x = vec[1];
  // we do not allow extracting variables for lvalues
  /*range-start*/$x[0]/*range-end*/ = vec[2];
}
