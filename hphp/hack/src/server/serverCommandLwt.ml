open Hh_prelude

let send_connection_type oc t =
  Marshal.to_channel oc t [];
  Out_channel.flush oc
