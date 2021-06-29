<?hh
namespace HH {
namespace FIXME {

/* This is a function meant to trace violations of type safety and provide
 * more information to the typechecker than existing error-suppression
 * mechanisms. It has no runtime effect. */
function UNSAFE_CAST<<<__Explicit>> Tin, <<__Explicit>> Tout>(Tin $t, ?\HH\FormatString<nothing> $msg = null): Tout;

}}
