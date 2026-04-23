<?hh

// Test that expression-dependent types referenced by the result of
// an await are NOT cleaned up from the tpenv. Models the real-world
// pattern: await $obj->genViewer() where genViewer() returns
// Awaitable<this::TViewer> on a type with an ABSTRACT type constant,
// producing an expression-dependent result.

interface IViewerForCleanupTest {
  public function getID(): int;
}

interface IContextForCleanupTest {
  abstract const type TViewer as IViewerForCleanupTest;

  public function genViewer(): Awaitable<this::TViewer>;
}

async function gen_context_for_cleanup_test(
): Awaitable<IContextForCleanupTest> {
  throw new Exception("stub");
}

async function test_cleanup_preserves_live_deps(): Awaitable<int> {
  $ctx = await gen_context_for_cleanup_test();

  // $ctx->genViewer() returns Awaitable<this::TViewer> where TViewer
  // is abstract. After await, $viewer has type <expr#N>::TViewer with
  // upper bound IViewerForCleanupTest. If cleanup deletes
  // <expr#N>::TViewer from tpenv, $viewer->getID() would fail with
  // "expression dependent type" error.
  $viewer = await $ctx->genViewer();
  return $viewer->getID();
}
