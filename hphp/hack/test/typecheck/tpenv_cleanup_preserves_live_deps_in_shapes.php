<?hh

// Test that expression-dependent types inside shape fields in the
// result of an await are preserved. Without proper traversal of all
// type constructors, the cleanup would miss expr-dep types nested
// inside shapes and delete them from the tpenv.

interface IMetadataForShapeCleanupTest {
  public function getKey(): string;
}

interface IProviderForShapeCleanupTest {
  abstract const type TMetadata as IMetadataForShapeCleanupTest;

  public function genResult(): Awaitable<
    shape('metadata' => this::TMetadata, 'status' => bool),
  >;
}

function make_provider_for_shape_cleanup_test(
): IProviderForShapeCleanupTest {
  throw new Exception("stub");
}

async function test_cleanup_preserves_deps_in_shapes(): Awaitable<string> {
  // The expression-dependent type is created INSIDE the await expression
  // (make_provider...() creates a fresh expr ID). After await, the shape
  // contains <expr#N>::TMetadata where N >= snapshot's nextid.
  $result = await make_provider_for_shape_cleanup_test()->genResult();

  // Accessing getKey() requires <expr#N>::TMetadata to still have its
  // upper bound IMetadataForShapeCleanupTest in the tpenv.
  return $result['metadata']->getKey();
}
