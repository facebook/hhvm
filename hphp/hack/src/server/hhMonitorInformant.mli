include HhMonitorInformant_sig.S

module Fake_informant : Informant_sig.S with type init_env = init_env

module Prefetcher : HhMonitorInformant_sig.Prefetcher_S
