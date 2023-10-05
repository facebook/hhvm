<?hh
function f() {
'√';
  g();
  // The --at-char API works in terms of byte offsets. We provide the offset of
  // the semicolon on line 4 (in statement `g();`), which is 32 bytes past the
  // start of the file. The resulting formatted range should be bytes 25-33,
  // from the semicolon on line 3 to the semicolon on line 4.
}
