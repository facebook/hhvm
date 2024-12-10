# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from create_salesforce_case_with_mail.create_salesforce_case_with_mail_draft_service.thrift_clients import (
    CreateSalesforceCaseWithMailDraftService,
)

from create_salesforce_case_with_mail.create_salesforce_case_with_mail_draft_service.thrift_types import (
    CreateSalesforceCaseWithMailDraftRequest,
    CreateSalesforceCaseWithMailDraftResponse_Thrift,
)
from gatekeeper.py.gk import GK
from libfb.py import employee

from servicerouter.python.sync_client import get_sr_client

GK_NAME = "care_platform_fbcode_thrift_case_creation_service"
TIER_NAME = "care_platform.create_case_with_mail_for_bse.fbcode_www_thrift_service"


def create_case_with_mail(request: CreateSalesforceCaseWithMailDraftRequest):
    if GK.check(GK_NAME, employee.get_current_unix_user_fbid()):
        with get_sr_client(
            CreateSalesforceCaseWithMailDraftService,
            tier=TIER_NAME,
        ) as client:
            return client.genCreateSalesforceCaseWithMailDraft(request)
    else:
        return CreateSalesforceCaseWithMailDraftResponse_Thrift()
