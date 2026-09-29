<?hh

// A string is falsy when it is "" or "0". HHBBC narrowed an unspecialized
// string to "" alone on the falsy branch, which let it prove that a write
// past the end -- which only the empty string fails at -- always throws, and
// mark the code after it unreachable. "0" then walked into that code and hit
// a StaticAnalysisError.
//
// "" is checked too, since that is the value the narrowing was right about
// and must keep behaving the same.

function write_past_end(string $s): string {
  if (!$s) {
    $s[2] = "x";
  }
  return $s;
}

function length_of(string $s): int {
  if (!$s) {
    return \strlen($s);
  }
  return -1;
}

<<__EntryPoint>>
function main_falsy_string_narrowing(): void {
  foreach (vec["0", "", "x"] as $s) {
    foreach (
      dict[
        'write' => write_past_end<>,
        'len' => length_of<>,
      ] as $name => $f
    ) {
      try {
        $r = (string)$f($s);
      } catch (\Throwable $e) {
        $r = \get_class($e);
      }
      echo $name, '(', $s, '): ', $r, "\n";
    }
  }
}
