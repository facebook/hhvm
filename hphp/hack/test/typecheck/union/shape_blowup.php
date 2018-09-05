<?hh //strict

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
function f(dict<string, mixed> $reason_arr): SentryRestrictionReasonThrown {
  $reason = shape();

  if (true) {
    $reason['original_restriction_class'] = "";
  }
  hh_show($reason);

  if (true) {
    $reason['aldrin_reason'] = "";
  }
  hh_show($reason);

  if (true) {
    $reason['feature'] = "";
  }
  hh_show($reason);

  if (true) {
    $reason['limited_fbid'] = 0;
  }
  hh_show($reason);

  if (true) {
    $reason['policy'] = (string)$reason_arr['policy'];
  }
  hh_show($reason);

  if (true) {
    $reason['admin'] = (int)$reason_arr['admin'];
  }

  if (true) {
    $reason['admin_id'] = (int)$reason_arr['admin_id'];
  }

  if (true) {
    $reason['flow'] = (int)$reason_arr['flow'];
  }

  if (true) {
    $reason['system'] = (int)$reason_arr['system'];
  }

  if (true) {
    $reason['reason'] = (string)$reason_arr['reason'];
  }

  if (true) {
    $reason['si_url_id'] = (int)$reason_arr['si_url_id'];
  }

  if (true) {
    $reason['karma_key'] = (string)$reason_arr['karma_key'];
  }

  if (true) {
    $reason['karma_key_that_hit'] = (string)$reason_arr['karma_key_that_hit'];
  }

  if (true) {
    $reason['allowance_spent_until'] = 1;
  }

  if (true) {
    $reason['filter'] = (string)$reason_arr['filter'];
  }

  if (true) {
    $reason['responsible_policies'] = "";
  }

  if (true) {
    $reason['urls'] = "";
  }

  if (true) {
    $reason['holdout_gks'] = "";
  }

  if (true) {
    $reason['expiration_time'] = (int)$reason_arr['expiration_time'];
  }

  if (true) {
    $reason['karma'] = (int)$reason_arr['karma'];
  }

  if (true) {
    $reason['bad_phrase'] = (string)$reason_arr['bad_phrase'];
  }

  if (true) {
    $reason['text_cluster_fbid'] = (int)$reason_arr['text_cluster_fbid'];
  }

  if (true) {
    $reason['manual_actor'] = (string)$reason_arr['manual_actor'];
  }

  if (true) {
    $reason['classifier'] = (string)$reason_arr['classifier'];
  }

  if (true) {
    $reason['corpus'] = (string)$reason_arr['corpus'];
  }

  if (true) {
    $reason['sample_id'] = (int)$reason_arr['sample_id'];
  }

  if (true) {
    $reason['target_feature'] = (int)$reason_arr['target_feature'];
  }

  if (true) {
    $reason['nonce'] = (int)$reason_arr['nonce'];
  }

  return $reason;
}
