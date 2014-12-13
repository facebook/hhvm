<?hh

/**
 * Basic async wrapper for curl
 *
 * Uses typical curl_multi winding and blocking technique
 * but with the block occuring in a separate thread
 * so that other actions can take place in other, non-curl awaitables.
 */
class AsyncCurl {
  /* cURL resources: Handles to process and multi-handle */
  protected array $handles = array();
  protected resource $multi;

  /* Internal state - Active handles and statue code */
  protected int $active;
  protected int $state = CURLM_OK;

  /* Perform actions until we would otherwise block */
  protected function windHandles(): void {
    if (empty($this->handles)) {
      $this->state = CURLM_OK;
      return;
    }

    do {
      $ret = curl_multi_exec($this->multi, $this->active);
    } while ($ret == CURLM_CALL_MULTI_PERFORM);
    $this->state = $ret;
  }

  /* Awaitable loop - Use curl_multi_await() to "block"
   * until there's work to do */
  async public function gen(): Awaitable<Vector<string>> {
    while ($this->active && $this->state == CURLM_OK) {
      // close-waiting-handle.php closes the multi handle on us
      // while we're in our await loop, so this can go false
      if (!is_resource($this->multi)) break;
      $select = await curl_multi_await($this->multi);
      if ($select == -1) {
        break;
      }
      $this->windHandles();
    }

    $ret = Vector {};
    foreach($this->handles as $handle) {
      $ret[] = (string)curl_multi_getcontent($handle);
    }
    return $ret;
  }

  /**
   * Initialize curl handles for basic GET urls
   * or pass in pre-prepared cURL easy handles to wind
   */
  public function __construct(...$sources): void {
    $this->multi = curl_multi_init();
    foreach ($sources as $source) {
      if (is_string($source)) {
        // URL
        $ch = curl_init($source);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_multi_add_handle($this->multi, $ch);
        $this->handles[] = $ch;
      } else if (is_resource($source)) {
        // Existing cURL handle
        curl_multi_add_handle($this->multi, $source);
        $this->handles[] = $source;
      } else {
        throw Exception("Unknown source type: " . gettype($source));
      }
    }
    $this->active = count($this->handles);
    $this->windHandles();
  }
}
