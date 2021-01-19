<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class WebServicesScalarType
extends WebServicesExposeAsType {
}

abstract class WebServicesExposeAsType {
  abstract const type TExposeAs;

  public function __construct(private ?this::TExposeAs $value = null) {
  }
  public function getValue(): ?this::TExposeAs {
    return $this->value;
  }
}

final class WebServicesIntType extends WebServicesScalarType {
  const type TExposeAs = int;
}

class SupplierConnectPVSServiceIL_RESPONSE {

  private ?WebServicesIntType $iLCODE;

  public function getILCODE(): ?int {
    return $this->iLCODE?->getValue();
  }

}
