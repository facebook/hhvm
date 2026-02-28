<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

interface IViewerContextBase {}

interface IEntUniverse {
  abstract const type TViewerContext as IViewerContextBase;
}

interface IAliteCoreDataBase {
  // concrete constant as alias to abstract constant
  const type TViewerContext = this::TEntUniverse::TViewerContext;
  abstract const type TEntUniverse as IEntUniverse;
}

interface IRequestBase {
  abstract const type TAliteCoreData as IAliteCoreDataBase;
}

interface IAliteRequestBase extends IRequestBase {
}

interface IXRequestBase {
  // Upper bound inherits abstract constant
  abstract const type TAliteRequest as IAliteRequestBase;
  // A concrete constant as an alias to an abstract constant
  const type TAliteCoreData = this::TAliteRequest::TAliteCoreData;
}

interface IAliteControllerBase {
  abstract const type TXRequest as IXRequestBase;
}

abstract class XControllerBase implements IAliteControllerBase {
  const type TConcreteRequest = this::TXRequest;
  // concrete constant as alias to another concrete constant which an alias to
  // an abstract constant introduced by an interface
  const type TViewerContext =
    this::TConcreteRequest::TAliteCoreData::TViewerContext;
}

interface ICometRouteMapperController {
  // require extends introduces TViewerContext
  require extends XControllerBase;
}

abstract class BaseCometRouteMapper {
  abstract const type TCometRouteMapperController as ICometRouteMapperController;
  // concrete constant as alias to abstract constant
  const type TViewerContext = this::TCometRouteMapperController::TViewerContext;
  public static async function genResponse(
    this::TViewerContext $_,
  ): Awaitable<void> {}
}

function expect(
  HH\FunctionRef<(readonly function<TViewerContext0 as IViewerContextBase>(
    TViewerContext0,
  ): Awaitable<void>)> $_,
): void {}

function testThisStuff(): void {
  $fptr = BaseCometRouteMapper::genResponse<>;
  expect($fptr);
}
