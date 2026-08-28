<?hh
// Regression test for T277621292: curl_getinfo() with an SLIST-type option
// (e.g. CURLINFO_COOKIELIST) leaked the curl_slist returned by
// curl_easy_getinfo on every call, because the traversal loop advanced the
// head pointer to null before handing it to curl_slist_free_all(). Repeated
// calls therefore grew process memory without bound.
//
// The slist is malloc'd by libcurl (not on the request heap), so the leak is
// only visible through memory_get_usage(true), which folds in jemalloc's
// per-thread allocation counters. No network access is required: seeding the
// cookie engine via CURLOPT_COOKIELIST is enough to make CURLINFO_COOKIELIST
// return a non-empty slist.

const int NUM_COOKIES = 50;
const int ITERATIONS = 20000;
// The buggy code leaks ~all NUM_COOKIES slist nodes plus their strdup'd cookie
// strings on every iteration -- tens of MB over the loop. A fixed handle
// leaks nothing, so allow a generous slack well above jemalloc's size-class /
// thread-cache noise but far below the buggy footprint.
const int MAX_GROWTH_BYTES = 4 * 1024 * 1024;

<<__EntryPoint>>
function main(): void {
  $ch = curl_init();
  for ($i = 0; $i < NUM_COOKIES; $i++) {
    // Netscape cookie-jar format: domain, includeSubdomains, path, secure,
    // expiry, name, value (tab separated). Far-future expiry keeps them live.
    curl_setopt(
      $ch,
      CURLOPT_COOKIELIST,
      ".example{$i}.com\tTRUE\t/\tFALSE\t2145916800\tname{$i}\tvalue{$i}",
    );
  }

  // Sanity guard: if seeding failed the slist would be empty, the leak would
  // never trigger, and this test would pass vacuously.
  $seeded = curl_getinfo($ch, CURLINFO_COOKIELIST);
  var_dump(is_array($seeded) && count($seeded) === NUM_COOKIES);

  // Warm up so lazy allocations / JIT translations settle before the baseline.
  for ($i = 0; $i < 1000; $i++) {
    curl_getinfo($ch, CURLINFO_COOKIELIST);
  }

  gc_collect_cycles();
  $baseline = memory_get_usage(true);

  for ($i = 0; $i < ITERATIONS; $i++) {
    curl_getinfo($ch, CURLINFO_COOKIELIST);
  }

  gc_collect_cycles();
  $growth = memory_get_usage(true) - $baseline;

  var_dump($growth < MAX_GROWTH_BYTES);

  curl_close($ch);
}
