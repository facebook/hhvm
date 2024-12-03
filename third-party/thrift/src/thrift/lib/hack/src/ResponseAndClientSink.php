<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

final class ResponseAndClientSink<
  TFirstResponse,
  TSinkType,
  TFinalResponseType,
> {
  public function __construct(
    public ?TFirstResponse $response,
    public (function(
      AsyncGenerator<null, TSinkType, void>,
    ): Awaitable<TFinalResponseType>) $genSink,
  ) {}
}
