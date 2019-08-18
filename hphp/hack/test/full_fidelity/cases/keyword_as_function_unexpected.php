<?hh

// Raise "Unexpected token"
function abstract() {}
function array() {}
function as() {}
function async() {}
function await() {}
function break() {}
function case() {}
function catch() {}
function class() {}
function clone() {}
function const() {}
function continue() {}
function default() {}
function do() {}
function echo() {}
function else() {}
function elseif() {}
function endfor() {}
function endforeach() {}
function endif() {}
function endswitch() {}
function endwhile() {}
function eval() {}
function extends() {}
function final() {}
function finally() {}
function for() {}
function foreach() {}
function function() {}
function global() {}
function goto() {}
function if() {}
function implements() {}
function include() {}
function include_once() {}
function inout() {}
function instanceof() {}
function insteadof() {}
function interface() {}
function list() {}
function namespace() {}
function new() {}
function print() {}
function private() {}
function protected() {}
function public() {}
function require() {}
function require_once() {}
function return() {}
function shape() {}
function static() {}
function switch() {}
function throw() {}
function trait() {}
function try() {}
function use() {}
function using() {}
function var() {}
function while() {}
function yield() {}

// abstract(); // can't test this; this token kicks us into parse_classish_declaration and makes the parser think the rest of the test is a classish body
array();
as();
async();
await();
break();
case();
catch();
// class(); // can't test this; this token kicks us into parse_classish_declaration and makes the parser think the rest of the test is a classish body
clone();
const();
continue();
declare();
default();
do();
echo();
else();
elseif();
endfor();
endforeach();
enddeclare();
endif();
endswitch();
endwhile();
eval();
extends();
// final(); // can't test this; this token kicks us into parse_classish_declaration and makes the parser think the rest of the test is a classish body
finally();
for();
foreach();
function();
global();
goto();
if();
implements();
include();
include_once();
inout();
instanceof();
insteadof();
// interface(); // can't test this; this token kicks us into parse_classish_declaration and makes the parser think the rest of the test is a classish body
list();
namespace();
new();
print();
private();
protected();
public();
require();
require_once();
return();
shape();
static();
// switch(); // can't test this; this token kicks us into parse_switch_statement and makes the parser think the rest of the test is a switch body
throw();
// trait(); // can't test this; this token kicks us into parse_classish_declaration and makes the parser think the rest of the test is a classish body
// try(); // can't test this; this token kicks us into parse_try_statement and makes the parser think the rest of the test is the body of a try
use();
using();
var();
while();
yield();
