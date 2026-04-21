<?hh

<<file:__EnableUnstableFeatures('class_aliases_everywhere')>>

// syntax error: attributes not allowed on class alias
<<MyAttribute>>
class C = D;
