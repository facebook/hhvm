<?hh // partial

class A implements HH\TypeParameterAttribute {}
class B implements HH\TypeParameterAttribute {}

function f<<<__Newable,__Enforceable,A>> reify T, >() {}
function g<T, <<Enforceable,__Newable>> reify Tu>() {}
function h<<<B>>T>() {}

function ff<<<A(1)>> T>() {}
