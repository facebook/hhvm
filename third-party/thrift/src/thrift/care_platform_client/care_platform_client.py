from create_salesforce_case_with_mail.create_salesforce_case_with_mail_draft_service.thrift_clients import (
    CreateSalesforceCaseWithMailDraftService,
)
from create_salesforce_case_with_mail.create_salesforce_case_with_mail_draft_service.thrift_types import (
    CreateSalesforceCaseWithMailDraftRequest,
)

from servicerouter.python.sync_client import get_sr_client


def create_case_with_mail(request: CreateSalesforceCaseWithMailDraftRequest):
    with get_sr_client(
        CreateSalesforceCaseWithMailDraftService,
        tier="care_platform.create_case_with_mail_for_bse.fbcode_www_thrift_service",
    ) as client:
        response = client.genCreateSalesforceCaseWithMailDraft(request)
