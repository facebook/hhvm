<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class ConfigChooser<-TContextProvider, +TResult> {

  public function __construct(
    private IConfigChooserProcessor<TContextProvider, TResult> $processor,
  ) {}

  public function getValue(TContextProvider $context_provider): TResult {
    throw new Exception();
  }
}

interface IConfigChooserProcessor<-TContextProvider, +TResult> {}
function getProductionModelID(
  IConfigChooserProcessor<KeyedTraversable<string, string>, string>
    $model_id_config_processor,
  ConstMap<string, mixed> $processor_map,
): string {
  $c = new ConfigChooser($model_id_config_processor);
  // We expect $c to have type ConfigChooser<v1, v2>
  // where IConfigChooserProcessorf<KeyedTraversable<string,string>, string> <: IConfigChooserProcessor<v1, v2>
  // so v1 <: KeyedTraversable<string,string> and string <: v2
  $v = $c->getValue($processor_map);
  // So now we have ConstMap<string,mixed> <: v1
  // and $v will have type v2
  // Finally we must therefore have v2 <: string
  // So clearly v2 = string
  // And by transitivity, ConstMap<string,mixed> <: KeyedTraversable<string,string>
  // By inheritance, KeyedTraversable<string,mixed> <: KeyedTraversable<string,string>
  // By covariance, mixed <: string
  // OOPS!!!!!
  return $v;
}
