/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace py3 create_salesforce_case_with_mail

# Response
struct CreateSalesforceCaseWithMailDraftResponse_Thrift {
  1: optional string case_id;
  2: optional string case_number;
  3: string reference_id;
}

# Request
struct CareContactInfo {
  1: string email;
  2: string full_name;
  3: optional string encrypted_user_id;
  4: optional string encrypted_datr;
  5: optional string phone_number;
}

union MixedValue {
  1: string string_value;
  2: i32 int_value;
  3: double double_value;
  4: bool bool_value;
}

enum CareCasePriority {
  HIGH = 0,
  LOW = 1,
}

enum CaseVisibility {
  PRIVATE = 0,
  PUBLIC = 1,
}

enum CareVertical {
  EC = 0,
  HELPDESK = 1,
  HRHOME = 2,
  HRHUB = 3,
  LEGALHUB = 4,
  MULESOFT_CLOUDHUB = 5,
  PAYMENT_PARTNER = 6,
  PEEPS = 7,
  PRIVACY_PORTAL = 8,
  RL = 9,
  SUPPLYCHAIN = 10,
  TEST_VERTICAL = 11,
}

struct Info {
  1: CareContactInfo contact_info;
  2: string subject;
  3: string description;
  4: optional map<string, MixedValue> additional_info;
  5: optional string locale;
  6: optional string case_channel;
  7: optional CareCasePriority care_priority;
  8: optional string external_case_id;
  9: optional string customer_internal_id;
  10: optional string case_owner_id;
  11: optional CaseVisibility case_visibility;
  12: optional string customer_id;
  13: optional string person_regarding_employee_id;
  14: bool async_response;
  15: optional string partner_vertical;
}

struct SubscribersInfo {
  1: string employee_id;
  2: string email_id;
}

struct EmailMessageInfo {
  1: optional string from_email_id;
  2: optional list<string> cc_email_ids;
  3: optional list<string> to_email_ids;
  4: optional list<string> bcc_email_ids;
  5: optional string subject;
  6: optional string body;
  7: optional string email_status;
}

struct CreateSalesforceCaseWithMailDraftRequest {
  1: optional string session_id;
  2: Info info;
  3: CareVertical vertical;
  4: string case_intake_classification_key;
  5: optional bool dry_run;
  6: optional list<SubscribersInfo> subscribers;
  7: optional EmailMessageInfo email_message_info;
}

service CreateSalesforceCaseWithMailDraftService {
  CreateSalesforceCaseWithMailDraftResponse_Thrift genCreateSalesforceCaseWithMailDraft(
    1: CreateSalesforceCaseWithMailDraftRequest request,
  );
}
