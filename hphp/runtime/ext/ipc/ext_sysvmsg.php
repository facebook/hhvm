<?hh

/**
 * The function converts the pathname of an existing accessible file and a
 *   project identifier into an integer for use with for example shmop_open()
 *   and other System V IPC keys.
 *
 * @param string $pathname - Path to an accessible file.
 * @param string $proj - Project identifier. This must be a one character
 *   string.
 *
 * @return int - On success the return value will be the created key value,
 *   otherwise -1 is returned.
 *
 */
<<__Native>>
function ftok(string $pathname, string $proj): int;

/**
 * msg_get_queue() returns an id that can be used to access the System V
 *   message queue with the given key. The first call creates the message queue
 *   with the optional perms. A second call to msg_get_queue() for the same key
 *   will return a different message queue identifier, but both identifiers
 *   access the same underlying message queue.
 *
 * @param int $key - Message queue numeric ID
 * @param int $perms - Queue permissions. Default to 0666. If the message
 *   queue already exists, the perms will be ignored.
 *
 * @return mixed - Returns a resource handle that can be used to access the
 *   System V message queue.
 *
 */
<<__Native>>
function msg_get_queue(int $key, int $perms = 0666): mixed;

/**
 * msg_queue_exists() checks whether a message queue exists
 *
 * @param int $key - Queue key.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function msg_queue_exists(int $key): bool;

/**
 * msg_send() sends a message of type msgtype (which MUST be greater than 0)
 *   to the message queue specified by queue.
 *
 * @param resource $queue
 * @param int $msgtype
 * @param mixed $message
 * @param bool $serialize - The optional serialize controls how the message is
 *   sent. serialize defaults to TRUE which means that the message is serialized
 *   using the same mechanism as the session module before being sent to the
 *   queue. This allows complex arrays and objects to be sent to other PHP
 *   scripts.
 * @param bool $blocking - If the message is too large to fit in the queue,
 *   your script will wait until another process reads messages from the queue
 *   and frees enough space for your message to be sent. This is called
 *   blocking; you can prevent blocking by setting the optional blocking
 *   parameter to FALSE, in which case msg_send() will immediately return FALSE
 *   if the message is too big for the queue, and set the optional errorcode to
 *   MSG_EAGAIN, indicating that you should try to send your message again a
 *   little later on.
 * @param mixed $errorcode
 *
 * @return bool - Returns TRUE on success or FALSE on failure.  Upon
 *   successful completion the message queue data structure is updated as
 *   follows: msg_lspid is set to the process-ID of the calling process,
 *   msg_qnum is incremented by 1 and msg_stime is set to the current time.
 *
 */
<<__Native>>
function msg_send(resource $queue,
                  int $msgtype,
                  mixed $message,
                  bool $serialize,
                  bool $blocking,
                  <<__OutOnly>>
                  inout mixed $errorcode): bool;

/**
 * msg_receive() will receive the first message from the specified queue of
 *   the type specified by desiredmsgtype.
 *
 * @param resource $queue
 * @param int $desiredmsgtype - If desiredmsgtype is 0, the message from the
 *   front of the queue is returned. If desiredmsgtype is greater than 0, then
 *   the first message of that type is returned. If desiredmsgtype is less than
 *   0, the first message on the queue with the lowest type less than or equal
 *   to the absolute value of desiredmsgtype will be read. If no messages match
 *   the criteria, your script will wait until a suitable message arrives on the
 *   queue. You can prevent the script from blocking by specifying
 *   MSG_IPC_NOWAIT in the flags parameter.
 * @param mixed $msgtype - The type of the message that was received will be
 *   stored in this parameter.
 * @param int $maxsize - The maximum size of message to be accepted is
 *   specified by the maxsize; if the message in the queue is larger than this
 *   size the function will fail (unless you set flags as described below).
 * @param mixed $message - The received message will be stored in message,
 *   unless there were errors receiving the message.
 * @param bool $unserialize - If set to TRUE, the message is treated as though
 *   it was serialized using the same mechanism as the session module. The
 *   message will be unserialized and then returned to your script. This allows
 *   you to easily receive arrays or complex object structures from other PHP
 *   scripts.  If unserialize is FALSE, the message will be returned as a
 *   binary-safe string.
 * @param int $flags - The optional flags allows you to pass flags to the
 *   low-level msgrcv system call. It defaults to 0, but you may specify one or
 *   more of the following values (by adding or ORing them together). Flag
 *   values for msg_receive MSG_IPC_NOWAIT If there are no messages of the
 *   desiredmsgtype, return immediately and do not wait. The function will fail
 *   and return an integer value corresponding to MSG_ENOMSG. MSG_EXCEPT Using
 *   this flag in combination with a desiredmsgtype greater than 0 will cause
 *   the function to receive the first message that is not equal to
 *   desiredmsgtype. MSG_NOERROR If the message is longer than maxsize, setting
 *   this flag will truncate the message to maxsize and will not signal an
 *   error.
 * @param mixed $errorcode - If the function fails, the optional errorcode
 *   will be set to the value of the system errno variable.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.  Upon
 *   successful completion the message queue data structure is updated as
 *   follows: msg_lrpid is set to the process-ID of the calling process,
 *   msg_qnum is decremented by 1 and msg_rtime is set to the current time.
 *
 */
<<__Native>>
function msg_receive(resource $queue,
                     int $desiredmsgtype,
                     <<__OutOnly("KindOfInt64")>>
                     inout mixed $msgtype,
                     int $maxsize,
                     <<__OutOnly>>
                     inout mixed $message,
                     bool $unserialize,
                     int $flags,
                     <<__OutOnly>>
                     inout mixed $errorcode): bool;

/**
 * msg_remove_queue() destroys the message queue specified by the queue. Only
 *   use this function when all processes have finished working with the message
 *   queue and you need to release the system resources held by it.
 *
 * @param resource $queue - Message queue resource handle
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function msg_remove_queue(resource $queue): bool;

/**
 * msg_set_queue() allows you to change the values of the msg_perm.uid,
 *   msg_perm.gid, msg_perm.mode and msg_qbytes fields of the underlying message
 *   queue data structure.  Changing the data structure will require that PHP be
 *   running as the same user that created the queue, owns the queue (as
 *   determined by the existing msg_perm.xxx fields), or be running with root
 *   privileges. root privileges are required to raise the msg_qbytes values
 *   above the system defined limit.
 *
 * @param resource $queue - Message queue resource handle
 * @param array $data - You specify the values you require by setting the
 *   value of the keys that you require in the data array.
 *
 * @return bool - Returns TRUE on success or FALSE on failure.
 *
 */
<<__Native>>
function msg_set_queue(resource $queue, darray $data): bool;

/**
 * msg_stat_queue() returns the message queue meta data for the message queue
 *   specified by the queue. This is useful, for example, to determine which
 *   process sent the message that was just received.
 *
 * @param resource $queue - Message queue resource handle
 *
 * @return array - The return value is an array whose keys and values have
 *   the following meanings: Array structure for msg_stat_queue msg_perm.uid The
 *   uid of the owner of the queue. msg_perm.gid The gid of the owner of the
 *   queue. msg_perm.mode The file access mode of the queue. msg_stime The time
 *   that the last message was sent to the queue. msg_rtime The time that the
 *   last message was received from the queue. msg_ctime The time that the queue
 *   was last changed. msg_qnum The number of messages waiting to be read from
 *   the queue. msg_qbytes The maximum number of bytes allowed in one message
 *   queue. On Linux, this value may be read and modified via
 *   /proc/sys/kernel/msgmnb. msg_lspid The pid of the process that sent the
 *   last message to the queue. msg_lrpid The pid of the process that received
 *   the last message from the queue.
 *
 */
<<__Native>>
function msg_stat_queue(resource $queue): darray;
