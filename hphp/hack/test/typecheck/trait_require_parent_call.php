<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class XMLTTagRenderer {

  abstract const type TXMLTType;
  protected async function genRenderPreviewTabs(
    this::TXMLTType $object,
  ): Awaitable<Map<string, int>> {
    return Map {};
  }

}

trait XMLTFBGamingVLCTagRendererWithValidation {
  require extends XMLTTagRenderer;

    <<__Override>>
      protected async function genRenderPreviewTabs(
        this::TXMLTType $object,
      ): Awaitable<Map<string, int>> {
    return await parent::genRenderPreviewTabs($object);
  }

}
