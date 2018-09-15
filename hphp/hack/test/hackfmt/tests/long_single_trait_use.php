<?hh

class C {
  // Don't break between keyword and single trait name, even though it would
  // bring this line under the length limit
  use TraitXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx;

  // Do break when multiple names are listed
  use TraitXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx, Y;
}
