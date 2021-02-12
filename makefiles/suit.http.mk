# Publish ID can be whatever the user prefer. The OTA server will take care of
# it. TODO: replace this by a JSON structure with metadata
SUIT_PUBLISH_ID ?= $(BOARD)_$(APPLICATION)

# Some requirements to publish and notify updates to the server
SUIT_OTA_SERVER_HOST ?= localhost
SUIT_OTA_SERVER_PORT ?= 8080
SUIT_OTA_SERVER_BASE_URL ?=
SUIT_OTA_SERVER_URL ?= http://$(SUIT_OTA_SERVER_HOST):$(SUIT_OTA_SERVER_PORT)$(SUIT_OTA_SERVER_BASE_URL)
SUIT_OTA_SERVER_COAP_URL_EP ?= coap/url

SUIT_NOTIFY_VERSION ?= latest

# The OTA server knows where the device can fetch the slots and manifest
SUIT_COAP_ROOT := $(shell curl -s -X GET \
      $(SUIT_OTA_SERVER_URL)/$(SUIT_OTA_SERVER_COAP_URL_EP)/$(SUIT_PUBLISH_ID))

suit/publish: $(SUIT_MANIFESTS) $(SLOT0_RIOT_BIN) $(SLOT1_RIOT_BIN)
	$(Q)curl -X POST \
		-F publish_id=$(SUIT_PUBLISH_ID) \
		-F $(SUIT_MANIFEST)=@$(SUIT_MANIFEST) \
		-F $(SUIT_MANIFEST_SIGNED)=@$(SUIT_MANIFEST_SIGNED) \
		-F $(SLOT0_RIOT_BIN)=@$(SLOT0_RIOT_BIN) \
		-F $(SLOT1_RIOT_BIN)=@$(SLOT1_RIOT_BIN) \
		$(SUIT_OTA_SERVER_URL)/publish

suit/notify: | $(filter suit/publish, $(MAKECMDGOALS))
	$(Q)curl -X POST \
		-F 'publish_id=$(SUIT_PUBLISH_ID)' \
		-F 'urls=$(SUIT_CLIENT)' \
		-F 'version=$(SUIT_NOTIFY_VERSION)' \
		$(SUIT_OTA_SERVER_URL)/notify
