<?hh // strict

namespace NS_shapes_rf_in_types_dir;

// ------------------------------------------------------------

class C {
  const string KEY1 = 'x';
  const int KEY2 = 3;
  const bool KEY3 = true;
  const num KEY4 = 2.5;
}

// ----------- Is a shape really a type? Yes, this has changed ------------

function CXf1(shape('x' => int, 'y' => int) $p1): void {}
function CXf2(): shape('x' => int, 'y' => int) { return shape('x' => 3, 'y' => 5); }
class CX {
  private ?shape('x' => int, 'y' => int) $p1 = null;
}

// Previously, we had to make a shape an alias before using it

type Point = shape('x' => int, 'y' => int);
function CYf1(Point $p1): void {}
function CYf2(): Point { return shape('x' => 10, 'y' => 12); }
class CY {
  private Point $p1 = shape('x' => 0, 'y' => 5);
}

function Point_toString(Point $p): string {
  return '(' . $p['x'] . ',' . $p['y'] . ')';
}

// It looks like the 2 forms of name can't be mixed
//function CYf3(): Point { return shape(C::KEY1 => 10, 'y' => 12); }

// Can the order of fields in the literal differ from the shape-specifier?
function CYf4(): Point { return shape('y' => 12, 'x' => 10); }	// apparently, YES

// Can a shape literal initialize fewer fields than exist? NO
//function CYf5(): Point { return shape('x' => 10); }	// The field 'y' is missing
//function CYf6(): Point { return shape('y' => 12); }	// The field 'x' is missing

//type PointNT = shape('x' => int, 'y' => int);
newtype PointNT = shape('x' => int, 'y' => int);

function PointNT_toString(PointNT $p): string {
  return '(' . $p['x'] . ',' . $p['y'] . ')';
}

function PointNT_getOrigin(): PointNT {
  return shape('x' => 0, 'y' => 0);
}

// ----------- Shape with no fields ------------

type st1 = shape();			// can have no fields

// ----------- Shapes with literal string keys ------------

type st2a = shape('x' => int);		// can have 1 field with '-quoted string key
//type st2b = shape('x'.'x' => int);	// gags: key must be a literal NOT an expression

//type st2c = shape("x" => int);	// gags on "-quoted string: Expected string literal or class constant
//type st2d = shape("x\tx\$" => int);	// gags too

/*
// can't use a heredoc literal
// hhvm says at <<< line: Fatal error: syntax error, unexpected T_START_HEREDOC, expecting ')'

type st2e = shape(
<<<ID
XXX
ID
 => int);
*/

/*
// can't use a nowdoc literal
// hhvm says at <<< line: Fatal error: syntax error, unexpected T_START_HEREDOC, expecting ')'

type st2f = shape(
<<<'ID'
YYY
ID
 => int);
*/

// ----------- Shapes with string keys, but using class constants ------------

type st3a = shape(C::KEY1 => int);	// OK; class constant of type string
function st3a_test(): st3a { return shape(C::KEY1 => 88); }

// Can I mix the 2 forms? Apparently not

//type st3b = shape(C::KEY1 => int, 'y' => int);	// Shape uses class constant as field name; But expected a literal string
//type st3c = shape('y' => int, C::KEY1 => int);	// Shape uses literal string as field name; But expected a class constant

// ----------- Shapes with int keys ------------

// There seems to be confusion about whether a key can have type int. The answer is NO!

//type st4a = shape(3 => int);		// gags: Expected string literal or class constant
type st4b = shape(C::KEY2 => int);	// but can have an int key via class constant!!!
//function st4b_test1(): st4b { return shape(3 => 99); }	// gags: Expected string literal or class constant
function st4b_test2(): st4b { return shape(C::KEY2 => 99); }	// but can do it through a class constant

//type st4c = shape('x' => int, 3 => int);	// gags: Expected string literal or class constant
//type st4d = shape('x' => int, C::KEY2 => int);	// gags: Shape uses class constant as field name;
						//       But expected a literal string

//type st5a = shape(true => int);	// gags: Expected string literal or class constant
//type st5b = shape(C::KEY3 => int);	// gags: A shape field name must be an int or string; Not a bool

//type st6a = shape(2.5 => int);	// gags: Expected string literal or class constant
//type st6b = shape(C::KEY4 => int);	// gags: A shape field name must be an int or string; Not a num

// ----------- Shapes with duplicate keys ------------

//type st7a = shape('abc' => int,'abc' => int);		// can't have duplicate keys
//type st7b = shape('cde' => int,'cde' => float);	// ditto

// ----------- Shapes containing exotic types (why not?) ------------

type st8 = shape('n1' => (int, float),'n2' => ?(function (array<num>): bool));

// ----------- Some examples of shape literals ------------

type Point3 = shape('x' => int, 'y' => int);
function getY(): int { return 123; }
function get_Point3(): Point3 {
  $prevX = 10;
  return shape('x' => $prevX, 'y' => getY());
}

type IdSet = shape('id' => ?string, 'url' => ?string, 'count' => int);
function get_IdSet(): IdSet {
  return shape('id' => null, 'url' => null, 'count' => 0);
}





