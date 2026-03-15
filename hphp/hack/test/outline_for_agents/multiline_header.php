<?hh

class ServiceController extends BaseController implements ILoggable {
  public function handleRequest(
    string $endpoint,
    dict<string, mixed> $params,
    ?ResponseFormatter $formatter,
    int $timeout_ms = 5000,
  ): Awaitable<ResponseObject> {
    // long method body
    return new ResponseObject();
  }

  <<__Override>>
  protected async function validateAndProcess(
    RequestContext $ctx,
    vec<InputRecord> $records,
  ): Awaitable<vec<ProcessedResult>> {
    // another long body
    return vec[];
  }

  private static function buildCacheKey(
    string $prefix,
    int $id,
    ?string $variant,
  ): string {
    return $prefix . ':' . $id;
  }
}

function process_batch(
  vec<Item> $items,
  (function(Item): bool) $filter,
  (function(Item): ProcessedItem) $transform,
): vec<ProcessedItem> {
  return Vec\map($items, $transform);
}
