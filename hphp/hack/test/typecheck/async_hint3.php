<?hh

// Testing function
async function wrong_hint(): varray<int> {
  throw new Exception();
}
