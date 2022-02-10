<?hh

function test(string $_cmd){
  $_test = csprintf("test %S", "abc123");
  $_test = csprintf(
    "extremely long string that the 80-cols linter will complain about".
    " therefore I decided to split into the concatenation of constant".
    " strings",
  );
}
