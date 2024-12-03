<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>> // @oss-disable
class ResponseAndStream<TResponse, TStreamResult> {
  public function __construct(
    public ?TResponse $response,
    public HH\AsyncGenerator<null, TStreamResult, void> $stream,
  ) {}

}
