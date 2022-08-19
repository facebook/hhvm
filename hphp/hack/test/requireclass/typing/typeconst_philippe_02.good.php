<?hh
// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file: __EnableUnstableFeatures('require_class')>>

interface IEntUniverse {
  abstract const type TViewerContext as IViewerContextBase;
}


interface IPAYEntMultiverse extends IEntUniverse {
  abstract const type TViewerContext as IPAYViewerContext;
}

interface ICROWEntMultiverse extends IEntUniverse {
  abstract const type TViewerContext as IPAYViewerContext;
}

final class FBEntUniverse implements IPAYEntMultiverse, ICROWEntMultiverse {
  const type TViewerContext = IFBViewerContext;
}


interface IViewerContextBase {}
interface IPAYViewerContext extends IViewerContextBase {}
interface ICROWViewerContext extends IViewerContextBase {}
interface IFBViewerContext extends IPAYViewerContext, ICROWViewerContext {}

interface IPajouxBase {
  abstract const type TUniverse as IEntUniverse;
  const type TVC = this::TUniverse::TViewerContext;
}


interface IPajouxA extends IPajouxBase {
  // IPAYViewerContext <: IViewerContextBase
  abstract const type TUniverse as IPAYEntMultiverse;
}

interface IPajouxB extends IPajouxBase {
  // ICROWViewerContext <: IViewerContextBase
  abstract const type TUniverse as ICROWEntMultiverse;
}

interface IPajouxFB extends IPajouxBase {
  const type TUniverse = FBEntUniverse;
}

abstract class PajouxFB implements IPajouxFB {}

final class Pajoux extends PajouxFB {
  // Since we extend PajouxFB, we know that:
  //  const type TVC = IFBViewerContext
  // and thus, we know that we can safely implement IPajouxA and IPajouxB.
  use TPajoux;
}

trait TPajoux implements IPajouxA, IPajouxB {
  // We require class Pajoux, so we should know that:
  //  const type TVC = IFBViewerContext
  require class Pajoux;

  public function foo(this::TVC $vc) : IFBViewerContext {
    return $vc;
  }

  public function bar(IFBViewerContext $vc) : this::TVC {
    return $vc;
  }
}
