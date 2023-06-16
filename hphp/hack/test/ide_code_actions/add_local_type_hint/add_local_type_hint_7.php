<?hh

function f() : void {
  // No refactoring should be offered as 'local_type_variables' is not enabled for this file.
  /*range-start*/$the_variable/*range-end*/ = 1;
}
