<?hh

class SerializationFormat {}

type AsyncJobOptions = shape(
  ?'serialization_format' => SerializationFormat,
  ?'execute_after' => int,
  ?'execute_before' => int,
  ?'expire_after' => int,
  ?'region' => string,
  ?'scheduling_uid' => int,
  ?'tag' => string,
  ?'retry_attempt' => int,
  ?'earlyreturn' => int,
  ?'_PIRANHA_SANDBOX_QUEUED_' => bool,
  ?'resource_requirement' => int,
  ?'execution_push_phase' => string,
  ?'async_bulkapi_factor' => float,
  ?'devserver' => string,
  ?'fifo_ordering_key' => string,
  ?'fifo_dedup_key' => string,
  ?'send_schedule_cats' => bool,
);


enum AsyncTierJobOption: string as string {
  // Used for delaying the execution of a job instance. Normally a job instance
  // would be ready for execution immediately after scheduling, and this option
  // adds an extra delay.
  EXECUTION_DELAY_SECONDS = 'execution_delay_seconds';

  // Sets a time-to-live (TTL) for expiring a scheduled job instance. If the
  // instance has not run by the time this TTL expires, it will be deleted.
  TTL_SECONDS = 'ttl_seconds';

  // Used to provide a preferred execution region for the job instance. This is
  // best-effort and the Async scheduler may run the job in a different region
  // regardless.
  REGION_PREFERENCE = 'region_preference';

  // Schedule a job instance to run through the production Async infrastructure,
  // but use the specified sandbox host for actual execution. This is typically
  // used for end-to-end testing of your job function. The value should be a
  // host:port combo, e.g. 'www.<username>.devvm1703.ash0.facebook.com:443'.
  SANDBOX_HOST = 'sandbox_host';

  // Early return option for the submitter service. This should be used
  // when the caller does not care about the async job id. This
  // will help return faster, hence, improving latency.
  // Batch enabled jobs will always early return.
  EARLY_RETURN = 'earlyreturn';

  // https://fburl.com/wiki/878tcoue
  // Boolean property which determines whether the client generates available
  // CATs such as from logged in users, web controllers, async job ids, etc.
  // These CATs are sent to the Async submitter for checking against
  // can_schedule ASYNC_JOB ACLs.
  SEND_SCHEDULE_CATS = "send_schedule_cats";

  IPF_PRODUCT_OVERRIDE = "ipf_product_override";

  // Legacy
  RETRY_ATTEMPT_DEPRECATED = "retry_attempt";

  // Used for Iris FIFO job types.
  // If the job type is an Iris FIFO job type, this MUST be set.
  // If the job type is not an Iris FIFO job type, this value will be ignored.
  FIFO_ORDERING_KEY = "fifo_ordering_key";
}


interface IPFOverrideParams {}

type AsyncTierJobOptions = shape(
  ?AsyncTierJobOption::EXECUTION_DELAY_SECONDS => int,
  ?AsyncTierJobOption::TTL_SECONDS => int,
  ?AsyncTierJobOption::REGION_PREFERENCE => string,
  ?AsyncTierJobOption::SANDBOX_HOST => string,
  ?AsyncTierJobOption::EARLY_RETURN => int,
  ?AsyncTierJobOption::SEND_SCHEDULE_CATS => bool,
  ?AsyncTierJobOption::IPF_PRODUCT_OVERRIDE => IPFOverrideParams,
  ?AsyncTierJobOption::RETRY_ATTEMPT_DEPRECATED => int,
  ?AsyncTierJobOption::FIFO_ORDERING_KEY => string,
);


class Stuff {
  public static async function genAsyncJob(
    ?AsyncTierJobOptions $options = null,
  ): Awaitable<string> {
    return "";
  }

  public static final async function genScheduleTestable(
    AsyncJobOptions $options = shape(),
  ): Awaitable<string> {
    return await self::genAsyncJob($options);
  }
}
