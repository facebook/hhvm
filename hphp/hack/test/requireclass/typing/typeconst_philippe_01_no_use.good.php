<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file: __EnableUnstableFeatures('require_class')>>

interface IViewerContextBase {}
interface IPAYViewerContext extends IViewerContextBase {}
interface ICROWViewerContext extends IViewerContextBase {}
interface IFBViewerContext extends IPAYViewerContext, ICROWViewerContext {}



interface IPajouxBase {
  abstract const type TVC as IViewerContextBase;
}

interface IPajouxA {
  // IPAYViewerContext <: IViewerContextBase
  abstract const type TVC as IPAYViewerContext;
}

interface IPajouxB {
  // ICROWViewerContext <: IViewerContextBase
  abstract const type TVC as ICROWViewerContext;
}

abstract class PajouxBase {
  abstract const type TVC as IViewerContextBase;
}

abstract class PajouxFB extends PajouxBase {
  // IFBViewerContext <: IPAYViewerContext
  // IFBViewerContext <: ICROWViewerContext
  const type TVC = IFBViewerContext;
}

final class Pajoux extends PajouxFB implements IPajouxA, IPajouxB {
  // Since we extend PajouxFB, we know that:
  //  const type TVC = IFBViewerContext
  // and thus, we know that we can safely implement IPajouxA and IPajouxB.
}

trait TPajoux implements IPajouxA, IPajouxB {
  // We require class Pajoux, so the typechecker knows that:
  //  const type TVC = IFBViewerContext

  require class Pajoux;

  public function foo(this::TVC $vc) : IFBViewerContext {
    return $vc;
  }

  public function bar(IFBViewerContext $vc) : this::TVC {
    return $vc;
  }
}
