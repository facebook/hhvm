<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

async function bad_async_return(): Awaitable<shape(...Left, ...Right)> {
  throw new Exception();
}
