<?hh // partial

// Testing function
async function wrong_hint(): varray<int> {
  throw new Exception();
}
