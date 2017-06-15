<?hh

// AUTOCOMPLETE 11 5

function foo(){
  try{
    functionThatMightFail();
  }
  catch (DivisionByZeroError $nonComprehensiveExceptionType) {
  }
  ca //This should only try to autocomplete catch and not case
