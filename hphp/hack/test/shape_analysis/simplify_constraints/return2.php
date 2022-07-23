<?hh

// Empty returns don't make the analysis crash
function f(): void {
  while (true) {
    return;
  }
  return;
}
