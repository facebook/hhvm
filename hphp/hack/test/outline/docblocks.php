<?hh

/*
  Multi line
  doc block
*/
function f1() {}

/* Newline between docblock and function */

function f2() {}

/* Too many newlines - this is not a docblock*/


function no_docblock1() {}
/* Narrow space */
function f4() {}
function no_docblock2() {}

// line comment
class C {

  /* Property */
  public string $x;

  // multi
  // line
  // line comment
  public function m1() {

  }
}
