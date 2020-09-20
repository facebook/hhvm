function foo(): vec<string> {
  // if we don't have a hack syntax tree, this is a parser error: it's trying
  // to access the constant `vec`, and index it as a list. lists aren't valid
  // subscript indices.
  return vec['foo', 'bar'];
}
