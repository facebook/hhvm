<?hh

// Raise "Unexpected token"
class abstract {}
class array {}
class as {}
class break {}
class case {}
class catch {}
class class {}
class clone {}
class const {}
class continue {}
class default {}
class do {}
class echo {}
class else {}
class elseif {}
class empty {}
class endfor {}
class endforeach {}
class endif {}
class endswitch {}
class endwhile {}
class eval {}
class extends {}
class final {}
class finally {}
class for {}
class foreach {}
class function {}
class global {}
class goto {}
class if {}
class implements {}
class include {}
class include_once {}
class inout {}
class instanceof {}
class insteadof {}
class interface {}
class isset {}
class list {}
class namespace {}
class new {}
class print {}
class private {}
class protected {}
class public {}
class require {}
class require_once {}
class return {}
class shape {}
class static {}
class switch {}
class throw {}
class trait {}
class try {}
class unset {}
class use {}
class using {}
class var {}
class while {}
class yield {}

new abstract();
new array();
new as();
new break();
new case();
new catch();
// new class(); // can't test this; error recovery makes us stop parsing the new expression when we see the class token; then we start parsing from the class token as if we have a classish declaration, and the entire rest of the test is seen as a classish body
new clone();
new const();
new continue();
new declare();
new default();
new do();
new echo();
new else();
new elseif();
new empty();
new endfor();
new endforeach();
new enddeclare();
new endif();
new endswitch();
new endwhile();
new eval();
new extends();
new final();
new finally();
new for();
new foreach();
new function();
new global();
new goto();
new if();
new implements();
new include();
new include_once();
new inout();
new instanceof();
new insteadof();
// new interface(); // can't test this; error recovery makes us stop parsing the new expression when we see the interface token; then we start parsing from the interface token as if we have a classish declaration, and the entire rest of the test is seen as a classish body
new isset();
new list();
new namespace();
new new();
new print();
new private();
new protected();
new public();
new require();
new require_once();
new return();
new shape();
new static();
new switch();
new throw();
// new trait(); // can't test this; error recovery makes us stop parsing the new expression when we see the trait token; then we start parsing from the trait token as if we have a classish declaration, and the entire rest of the test is seen as a classish body
new try();
new unset();
new use();
new using();
new var();
new while();
new yield();
