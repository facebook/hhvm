<?hh

// Currently, the quickfix applying tool cannot handle, nested patches, so it
// drops the one that encloses the other.
function nested_quickfix(): void {
  (float) ((float) 42.0 + (float) 24.0);
}
