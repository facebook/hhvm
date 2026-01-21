<?hh

const int MY_CONST = 1;

function test(): void {
  // Using a constant instead of a variable in foreach value
  foreach (vec[1, 2, 3] as MY_CONST) {
  }
}
