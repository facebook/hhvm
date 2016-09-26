(** Notifer results come in 3 flavors:
 *
 * 1) Notifier is not available (Dfind not available or watchman
 *    subscription is down)
 * 2) Synchronous changes contain all changes up to the point
 *    that the notifier was invoked
 * 3) Asynchronous changes include whatever changes that have been
 *    pushed up to this point.
 * *)
type notifier_changes =
  | Notifier_unavailable
  | Notifier_synchronous_changes of SSet.t
  | Notifier_async_changes of SSet.t
