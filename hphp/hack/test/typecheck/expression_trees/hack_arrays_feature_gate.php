<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test_vec_without_feature(): void {
  ExampleDsl`vec[1, 2, 3]`;
}

function test_dict_without_feature(): void {
  ExampleDsl`dict["a" => 1]`;
}

function test_keyset_without_feature(): void {
  ExampleDsl`keyset[1, 2, 3]`;
}
