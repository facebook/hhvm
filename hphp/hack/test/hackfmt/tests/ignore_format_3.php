<?hh

function test(): void {

// It's possible to format the following in such a way that would make
// the ignore comment to appear in the trailing trivia after formatting,
// instead of the leading trivia, where it appeared before. This would
// create a situation where formatting the document would not be idempotent.
$x =
/*hackfmt-ignore*/ (3+1)
;

}
