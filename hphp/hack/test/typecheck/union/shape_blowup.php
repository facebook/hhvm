<?hh

class SentryExceptionReason {}

type SentryRestrictionReasonThrown = shape(
  ?'original_restriction_class' => ?string,
  ?'aldrin_reason' => ?string,
  ?'feature' => ?string,
  ?'limited_fbid' => ?int,
  ?'policy' => ?string,
  ?'admin' => ?int,
  ?'admin_id' => ?int,
  ?'flow' => ?int,
  ?'system' => ?int,
  ?'reason' => ?string,
  ?'block_on_error_exception' => ?SentryExceptionReason,
  ?'log_on_error_exception' => ?SentryExceptionReason,
  ?'si_url_id' => ?int,
  ?'karma_key' => ?string,
  ?'karma_key_that_hit' => ?string,
  ?'xargs' => ?darray<arraykey, mixed>,
  ?'allowance_spent_until' => ?int,
  ?'filter' => ?string,
  ?'responsible_policies' => ?string,
  ?'urls' => string,
  ?'holdout_gks' => string,
  ?'expiration_time' => ?int,
  ?'karma' => ?int,
  ?'page_blacklist' => ?varray<mixed>,
  ?'photo_cluster_block_reason' => ?SentryExceptionReason,
  ?'bad_phrase' => ?string,
  ?'text_cluster_fbid' => ?int,
  ?'manual_actor' => ?string,
  ?'classifier' => ?string,
  ?'corpus' => ?string,
  ?'sample_id' => ?int,
  ?'sigma_reason_xargs' => ?darray<arraykey, mixed>,
  ?'target_feature' => ?int,
  ?'nonce' => ?int,
  ...
);

// The infered type for $reason used to grow exponentially
function f(
  bool $b,
  dict<string, mixed> $reason_arr
): SentryRestrictionReasonThrown {

  $reason = shape();

  if ($b) {
    $reason['original_restriction_class'] = "";
  }
  hh_show($reason);

  if ($b) {
    $reason['aldrin_reason'] = "";
  }
  hh_show($reason);

  if ($b) {
    $reason['feature'] = "";
  }
  hh_show($reason);

  if ($b) {
    $reason['limited_fbid'] = 0;
  }
  hh_show($reason);

  if ($b) {
    $reason['policy'] = (string)$reason_arr['policy'];
  }
  hh_show($reason);

  if ($b) {
    $reason['admin'] = (int)$reason_arr['admin'];
  }

  if ($b) {
    $reason['admin_id'] = (int)$reason_arr['admin_id'];
  }

  if ($b) {
    $reason['flow'] = (int)$reason_arr['flow'];
  }

  if ($b) {
    $reason['system'] = (int)$reason_arr['system'];
  }

  if ($b) {
    $reason['reason'] = (string)$reason_arr['reason'];
  }

  if ($b) {
    $reason['si_url_id'] = (int)$reason_arr['si_url_id'];
  }

  if ($b) {
    $reason['karma_key'] = (string)$reason_arr['karma_key'];
  }

  if ($b) {
    $reason['karma_key_that_hit'] = (string)$reason_arr['karma_key_that_hit'];
  }

  if ($b) {
    $reason['allowance_spent_until'] = 1;
  }

  if ($b) {
    $reason['filter'] = (string)$reason_arr['filter'];
  }

  if ($b) {
    $reason['responsible_policies'] = "";
  }

  if ($b) {
    $reason['urls'] = "";
  }

  if ($b) {
    $reason['holdout_gks'] = "";
  }

  if ($b) {
    $reason['expiration_time'] = (int)$reason_arr['expiration_time'];
  }

  if ($b) {
    $reason['karma'] = (int)$reason_arr['karma'];
  }

  if ($b) {
    $reason['bad_phrase'] = (string)$reason_arr['bad_phrase'];
  }

  if ($b) {
    $reason['text_cluster_fbid'] = (int)$reason_arr['text_cluster_fbid'];
  }

  if ($b) {
    $reason['manual_actor'] = (string)$reason_arr['manual_actor'];
  }

  if ($b) {
    $reason['classifier'] = (string)$reason_arr['classifier'];
  }

  if ($b) {
    $reason['corpus'] = (string)$reason_arr['corpus'];
  }

  if ($b) {
    $reason['sample_id'] = (int)$reason_arr['sample_id'];
  }

  if ($b) {
    $reason['target_feature'] = (int)$reason_arr['target_feature'];
  }

  if ($b) {
    $reason['nonce'] = (int)$reason_arr['nonce'];
  }

  return $reason;
}
