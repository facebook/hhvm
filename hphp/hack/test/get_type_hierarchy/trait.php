<?hh

trait Trait_1 {

}
trait Trait_2 {
  use Trait_1;
}

trait Trait_3 {
  use Trait_2;
  //    ^ type-hierarchy-at-caret
}
