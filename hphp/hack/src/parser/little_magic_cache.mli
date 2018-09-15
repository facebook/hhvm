type ('k, 'v) t
val memoize : ('k, 'v) t -> ('k -> 'v) -> ('k -> 'v)
val make : unit -> ('k, 'v) t
