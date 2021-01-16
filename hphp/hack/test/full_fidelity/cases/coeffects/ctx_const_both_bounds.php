<?hh
abstract class AtLeastRxShallowButImpureIterator {
  abstract const ctx C super [rx_shallow] as [rx] = [rx];

  public function call_into_rx()[this::C]: void {
    require_rx(); // ok (rx_shallow <: C <: rx)
  }
}

function require_rx()[rx]: void {}

class ExactlyRxIterator extends AtLeastRxShallowButImpureIterator {
  const ctx C super [rx_shallow] as [rx] = [rx];
}

class ExactlyRxShallowIterator extends AtLeastRxShallowButImpureIterator {
  const ctx C super [rx_shallow] as [rx] = [rx_shallow];
  public function call_into_rx_shallow()[this::C]: void {}
}
