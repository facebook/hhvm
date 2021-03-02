<?hh

/* This is a function meant to trace violations of type safety and provide
 * more information to the typechecker than existing error-suppression
 * mechanisms. It has no runtime effect. */
function unsafe_cast<<<__Explicit>> Tin, <<__Explicit>> Tout>(Tin $t): Tout;
