//// def.php
<?hh
<<file:__EnableUnstableFeatures(
  'newtype_super_bounds',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

newtype Wrapped<-T as shape(...)>
  as shape(...)
  super shape(...T, ?'dummy' => nothing) = shape(...);

//// use.php
<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

interface Box<+T as shape(...)> {
  public function acceptor(): (function(
    shape(...Wrapped<T>, ?'q' => bool),
  ): void);
}

function box<T as shape(...)>(T $_): Box<T> {
  throw new Exception();
}

function test<T as shape(...)>(bool $condition, T $generic): void {
  // Covariance preserves the inferred union as Box's type argument. Wrapped's
  // super bound then places that union in splat position.
  $box = $condition ? box($generic) : box(shape('a' => 1));
  $accept = $box->acceptor();
  hh_show($accept);
  $accept(shape('a' => 42));
}
