<?hh

function my_json_decode(string $_s): dynamic {
  return 0 as dynamic;
}

class MyRequest {
  public int $appID = 0;
  public ?string $method = null;
  public ?int $userID = null;
  public mixed $params = null;
  public dict<string, string> $headers = dict[];
  public ?string $sourceIP = null;
  public ?int $carrierID = null;

  public static function fromJSON(string $request_json): MyRequest {
    $data = my_json_decode($request_json);
    invariant(
      $data is KeyedContainer<_, _>,
      'Expecting a Map',
    );

    $request = new MyRequest();
    $request->appID = (int)idx($data, 'app_id', 0);
    $request->method = HH\FIXME\UNSAFE_CAST<mixed, ?string>(
      idx($data, 'method'),
    );
    $request->userID = null;
    $request->params = $data['request']['parameters'];
    $request->headers = $data['request']['headers'];
    $request->sourceIP = HH\FIXME\UNSAFE_CAST<mixed, ?string>(
      idx($data, 'source_ip'),
    );
    $request->carrierID = HH\FIXME\UNSAFE_CAST<mixed, ?int>(
      idx($data, 'carrier_id'),
    );

    return $request;
  }
}
