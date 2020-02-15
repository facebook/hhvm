<?hh

// We expect some typechecker errors in this one because we want the decl parser
// to be more accepting than the typechecker.

const string MY_CONST = "my const";
const int MY_CONST2 = 5;
const float MY_CONST3 = 5.5;
const num MY_CONST5 = 5e1;
const bool MY_CONST6 = true;
const vec<int> MY_VEC = vec[];
