#!/bin/bash

# This test makes sure that hh_client lsp respects delayUntilDoneInit

HH_CLIENT="$1"
NAMING_TABLE=$(realpath "$2")
ROOT=$(realpath "$3")
FILE=$(realpath "$3/foo_1.php")  # this is a file which I know exists in simple-repo
cd "$ROOT" || exit 1  # because "hh_client lsp" must be launched from root

{
CMD1=$(cat <<EOF
  {
    "jsonrpc": "2.0",
    "id":1,
    "method":"initialize",
    "params": {
      "rootPath": "$ROOT",
      "rootUri": "file://$ROOT",
      "initializationOptions": {
        "namingTableSavedStatePath": "$NAMING_TABLE",
        "namingTableSavedStateTestDelay": 1.0,
        "delayUntilDoneInit": true
      },
      "capabilities": {
        "workspace": {
          "didChangeWatchedFiles": {
            "dynamicRegistration": true
          }
        }
      }
    }
  }
EOF
)
printf "Content-Length: %s\r\n\r\n%s" "${#CMD1}" "$CMD1"

CMD3=$(cat <<EOF
      {
        "jsonrpc": "2.0",
        "method": "textDocument/didOpen",
        "params": {
          "textDocument": {
            "uri": "file://$FILE",
            "languageId": "hack",
            "version": 1,
            "text": $(jq --slurp --raw-input . "$FILE")
          }
        }
      }
EOF
)
printf "Content-Length: %s\r\n\r\n%s" "$(printf "%s" "$CMD3" | wc -c)" "$CMD3"

CMD4=$(cat <<EOF
      {
        "jsonrpc": "2.0",
        "id": 2,
        "method": "textDocument/hover",
        "params": {
          "textDocument": {
            "uri": "file://$FILE"
          },
          "position": {
            "line": 0,
            "character": 0
          }
        }
      }
EOF
  )
printf "Content-Length: %s\r\n\r\n%s" "${#CMD4}" "$CMD4"

CMD98='{"jsonrpc":"2.0", "id":98, "method":"shutdown"}'
printf "Content-Length: %s\r\n\r\n%s" "${#CMD98}" "$CMD98"

CMD99='{"jsonrpc":"2.0", "method":"exit"}'
printf "Content-Length: %s\r\n\r\n%s" "${#CMD99}" "$CMD99"

} | "$HH_CLIENT" lsp | tee >(cat 1>&2) | grep '"id":2,"result":null'
# Here we're sending the entire LSP series of requests into the stdin of "hh_client lsp",
# and verifying that its responses over stdout include response "null" to our hover request id 2.
# If grep fails, then it will have exit code 1, and our test runner will likely
# display all of stdout+stderr. Well, the stdout lsp responses have been gobbled up by grep,
# but we used "tee" to duplicate them into stderr, so we'll get that in case of failure.
