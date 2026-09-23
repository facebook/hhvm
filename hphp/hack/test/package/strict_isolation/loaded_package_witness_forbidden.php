//// isolated/target.php
<?hh

function strict_target(): void {}

//// standalone/unrelated.php
<?hh

function unrelated_caller(): void {
  if (package strict_deployment) {
    strict_target(); // error: a deployment witness does not grant access
  }
}

//// strict_soft_consumer/soft.php
<?hh

function soft_including_caller(): void {
  if (package strict_deployment) {
    strict_target(); // error: a deployment witness does not upgrade a soft include
  }
}

//// strict_hard_consumer/hard.php
<?hh

function hard_including_caller(): void {
  if (package strict_deployment) {
    strict_target(); // ok: the caller includes the strict package
  }
}
