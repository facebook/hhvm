<?hh

// This test makes sure that even if entrypoint-symlink2 and entrypoint-symlink3
// exists we can still build the wrapper. Because instead of them adding this file
// more than once we create an EntryPoint wrapper that calls the function in the
// target file

<<__EntryPoint>>
function main() :mixed{
  echo 'HI!';
}
