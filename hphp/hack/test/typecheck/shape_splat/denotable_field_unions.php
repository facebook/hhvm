<?hh
<<file:__EnableUnstableFeatures(
  'like_type_hints',
  'shape_splat_concrete',
)>>

type Left = shape(
  'arraykey' => int,
  'num' => int,
  'nullable' => string,
);
type Right = shape(
  ?'arraykey' => string,
  ?'num' => float,
  ?'nullable' => null,
);

type Good = shape(...Left, ...Right);

// An optional field is widened with the unknown value from an earlier open
// row. The resulting `bool | mixed` simplifies to `mixed`.
type Open = shape(...);
type Optional = shape(?'open_widened' => bool);
type OpenWidened = shape(...Open, ...Open, ...Optional);

type RequiredLike = shape('field' => ~mixed);
type RequiredNestedLike = shape('field' => Traversable<~int>);
type RequiredOpenShape = shape('field' => shape(...));
type NestedAlias = shape(...shape('inner' => bool), ...shape('inner' => ~mixed));
type RequiredNestedAlias = shape('field' => vec<NestedAlias>);

type LikeWins = shape(...shape('field' => bool), ...RequiredLike);
type NestedLikeWins = shape(...shape('field' => bool), ...RequiredNestedLike);
type OpenShapeWins = shape(...shape('field' => bool), ...RequiredOpenShape);
type NestedAliasWins = shape(...shape('field' => bool), ...RequiredNestedAlias);
