<?hh

function main(): void {
  // Ambiguity with heredocs
  $var = prefix<<<"MYLABEL"
}
