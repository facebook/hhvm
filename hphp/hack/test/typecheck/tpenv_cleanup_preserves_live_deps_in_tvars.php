<?hh

// Test that expression-dependent types inside solved type variables
// in the result of an await are preserved. The result type may be
// a Tvar that resolves to a type containing expr-dep types; the
// cleanup must expand type variables to find them.

interface IItemForTvarCleanupTest {
  abstract const type TID;

  public function getID(): this::TID;
}

interface IQueryForTvarCleanupTest {
  abstract const type TItem as IItemForTvarCleanupTest;

  public function genItems(): Awaitable<vec<this::TItem>>;
}

function make_query_for_tvar_cleanup_test(): IQueryForTvarCleanupTest {
  throw new Exception("stub");
}

async function test_cleanup_preserves_deps_in_tvars(): Awaitable<void> {
  // The query chain creates expression-dependent types. After await,
  // the result type may be behind a solved Tvar containing
  // <expr#N>::TItem. If the cleanup doesn't expand Tvars, it
  // misses the live expr-dep types and deletes them.
  $items = await make_query_for_tvar_cleanup_test()->genItems();

  // Accessing getID() on elements requires <expr#N>::TItem and
  // <expr#N>::TItem::TID to still be in the tpenv.
  foreach ($items as $item) {
    $item->getID();
  }
}
