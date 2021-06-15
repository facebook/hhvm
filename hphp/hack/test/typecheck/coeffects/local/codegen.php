<?hh

function io_disallowed()[codegen]: void {
  echo "bad (missing `IO` capability)"; // error
}
