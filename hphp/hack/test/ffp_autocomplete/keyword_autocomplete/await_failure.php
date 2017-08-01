<?hh

/* AUTOCOMPLETE 6 9 */

function do_synchronous_things() {
  $x = a
  /*
   * This will return "async" but not "await" since we are not in an async
   * function
   */
