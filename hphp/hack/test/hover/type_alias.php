<?hh

/**
 * Some docs.
 */
type myfoo = int;

function return_it(): myfoo {
  //                     ^ hover-at-caret
  throw new Exception();
}
