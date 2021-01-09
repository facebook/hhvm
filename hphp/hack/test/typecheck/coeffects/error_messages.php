<?hh

function local()[]: void {
  echo "bad";
}

function caller()[]: void {
         callee();        }
function callee(): void {}
