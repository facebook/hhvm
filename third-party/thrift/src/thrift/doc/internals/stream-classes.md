# Stream classes

The class diagram for the stream classes is shown below:

```
*===========================*                     *============================*
| InputStreamCallback<S, T> |                     | OutputStreamCallback<S, T> |
*===========================*                     *============================*
 |                                                                           |
 |-inherits from                                               inherits from-|
 |                                                                           |
 |                                                                           |
 |  +-------------------------------+     +--------------------------------+ |
 |  | SyncInputStreamCallback<S, T> |     | SyncOutputStreamCallback<S, T> | |
 |  +-------------------------------+     +--------------------------------+ |
 |       |                ^  |                    |  ^                 |     |
 |       |-inherits from  |  |-uses          uses-|  |   inherits from-|     |
 |       |                |  |                    |  |                 |     |
 |       |        creates-|  |                    |  |-creates         |     |
 |       |                |  V                    V  |                 |     |
 |       |         *================*     *=================*          |     |
 |       |         | InputStream<T> |     | OutputStream<T> |          |     |
 |       |         *================*     *=================*          |     |
 |       |            |                                  |             |     |
 |       |            |-uses                        uses-|             |     |
 |       |            |                                  |             |     |
 |       V            V                                  V             |     |
 |  / - - - - - - - - - - - - - - - -\   / - - - - - - - - - - - - - - - - \ |
 |  | SyncInputStreamCallbackBase<T> |   | SyncOutputStreamCallbackBase<T> | |
 |  \ - - - - - - - - - - - - - - - -/   \ - - - - - - - - - - - - - - - - / |
 |       |                                                             |     |
 |       |-inherits from                                 inherits from-|     |
 |       |                                                             |     |
 |       V                                                             V     |
 |   +-------------------------+               +--------------------------+  |
 \-> | InputStreamCallbackBase |------\  /-----| OutputStreamCallbackBase |<-/
     +-------------------------+      |  |     +--------------------------+
                      |   *^          |  |          ^*   |
                 uses-|    |     uses-|  |-uses     |    |-uses
                      |    |          |  |          |    |
                      |    |-uses     |  |     uses-|    |
                      |    |-owns     |  |     owns-|    |
                      V   1|          |  |          |1   V
                 +--------------+     |  |     +------------+
        /--------| StreamSource |--\  |  |  /--| StreamSink |---------\
        |        +--------------+  |  |  |  |  +------------+         |
        |-uses        ^            |  |  |  |           ^        uses-|
        |-owns        |       uses-|  |  |  |-uses      |        owns-|
        |             |-uses       |  |  |  |      uses-|             |
        |             |-owns       |  |  |  |      owns-|             |
        V             |            V  V  V  V           |             V
 +--------------+     |         +---------------+       |     +--------------+
 | StreamReader |     \---------| StreamManager |-------/     | StreamWriter |
 +--------------+               +---------------+             +--------------+
                                  |         |
                                  |         |
                         /--------/         \--------------\
                         |                                 |
                         |-uses                       uses-|
                         |                                 |
                         V                                 V
              / - - - - - - - - - - - \              +-----------+
              | StreamChannelCallback |              | EventBase |
              \ - - - - - - - - - - - /              +-----------+


Legend:
     / - - - - - \              +-------+          +=====================+
     | Interface |              | Class |          | ClassExposedToUsers |
     \ - - - - - /              +-------+          +=====================+
```
