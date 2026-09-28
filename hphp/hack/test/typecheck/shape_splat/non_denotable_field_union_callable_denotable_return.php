<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type IntField = shape('field' => int);
type StringField = shape(?'field' => string);

function denotable_union(): shape(...IntField, ...StringField) {
  throw new Exception();
}
