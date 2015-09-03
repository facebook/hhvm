//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class PlaceClaimErrors {

  public final static function getTranslatedErrorMessages(int $error_type):
    :fbt {
    switch ($error_type) {
      // Arranged in alphabetical order
      case PlaceClaimErrorMessages::CALL_CANT_BE_SCHEDULED:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when we fail to make a call">
            We{"'"}re unable to complete this call right now.
            Please try again later.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::CHECKBOX_NOT_CHECKED:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user clicks on Continue without
                  checking the required checkbox">
            Confirm you{"'"}re an official representative of this Page to
            continue.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::CLAIM_REQUEST_UNSUCCESSFUL:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when claim request fails form our side">
            We weren{"'"}t able to submit your claim request. Please try again.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::FILE_TYPE_INCOMPATIBLE:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user uploads a file of incompatible
                  type which we don't support">
            This file type isn{"'"}t supported, please upload a compatible file.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::INVALID_PHONE_NUMBER:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user enters invalid phone number">
            This isn{"'"}t a valid phone number. Please check the number and
            try again.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::INVALID_PIN:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user enters wrong verification
                  PIN">
            The PIN you entered is incorrect. Please try again or request
            another call.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::MERGE_UNSUCCESSFUL:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when merge request fails">
            We were unable to complete your merge request. Please try again.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::NO_FILE_CHOSEN:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user hits Continue without
                  submitting any documents">
            You need to upload one of the documents listed to continue.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::NO_OPTION_SELECTED:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when user doesn't choose any option
                  from a given list">
             You need to choose an action to continue.
          </fbt>;
        break;
      case PlaceClaimErrorMessages::UNABLE_TO_UPLOAD:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message to show when we fail to upload user's files
                  on our servers">
            Something went wrong and your file wasn{"'"}t uploaded. Please try
            again.
          </fbt>;
        break;

      case PlaceClaimErrorMessages::RATE_LIMITED:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message when robocall rate limit is hit">
            Sorry, something went wrong with phone verification. Please verify
            by email or documentation instead.
          </fbt>;
        break;

      case PlaceClaimErrorMessages::RATE_LIMITED_EMAIL_ONLY:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Error message when robocall rate limit is hit and
              only email is avilable">
            Sorry, something went wrong with phone verification. Please verify
            by email instead.
          </fbt>;
        break;

      default:
        return
          <fbt
            project={PlaceClaimGeneralConstants::PROJECT_NAME}
            desc="Default error message to show when we don't know the error">
            Something went wrong. Please try again later.
          </fbt>;
        break;
    }
  }
}

//// pattern.php
<?hh //strict

class __KSTAR {}

class __SOMENODE {
  public function __KSTAR(): void {}

  public function __SOMENODE(): string {
    "__SKIPANY";
    {
      "__KSTAR";
      return "__ANY";
      break;
    }
  }

  public function __KSTAR(): void {}
}

class __KSTAR {}

//// target.php
<?hh //strict

class __KSTAR {}

class __SOMENODE {
  public function __KSTAR(): void {}

  public function __SOMENODE(): string {
    "__SKIPANY";
    {
      "__KSTAR";
      return "__ANY";
      /*DELETE STMTS*//**/
    }
  }

  public function __KSTAR(): void {}
}

class __KSTAR {}
