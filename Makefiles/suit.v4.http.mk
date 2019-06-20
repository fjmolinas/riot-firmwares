# Publish ID can be whatever the user prefer. The OTA server will take care of
# it. TODO: replace this by a JSON structure with metadata
SUIT_PUBLISH_ID ?= $(BOARD)_$(APPLICATION)

# Some requirements to publish and notify updates to the server
SUIT_COAP_SERVER ?= localhost
SUIT_OTA_SERVER_URL ?= http://$(SUIT_COAP_SERVER):8080
SUIT_OTA_SERVER_COAP_URL_EP ?= coap/url

# The OTA server knows where the device can fetch the slots and manifest
SUIT_COAP_ROOT ?= $(shell curl -X GET \
      $(SUIT_OTA_SERVER_URL)/$(SUIT_OTA_SERVER_COAP_URL_EP)/$(SUIT_PUBLISH_ID))

suit/manifest: $(SUIT_MANIFEST)

suit/publish: $(SUIT_MANIFEST) $(SUIT_MANIFEST_SIGNED) $(SLOT0_RIOT_BIN) $(SLOT1_RIOT_BIN)
	$(Q)curl -X POST \
		-F publish_id=$(SUIT_PUBLISH_ID) \
		-F $(SUIT_MANIFEST)=@$(SUIT_MANIFEST) \
		-F $(SUIT_MANIFEST_SIGNED)=@$(SUIT_MANIFEST_SIGNED) \
		-F $(SLOT0_RIOT_BIN)=@$(SLOT0_RIOT_BIN) \
		-F $(SLOT1_RIOT_BIN)=@$(SLOT1_RIOT_BIN) \
		$(SUIT_OTA_SERVER_URL)/publish

suit/notify:
	$(Q)curl -X POST \
		-F 'publish_id=$(SUIT_PUBLISH_ID)' \
		-F 'urls=$(SUIT_CLIENT)' \
		$(SUIT_OTA_SERVER_URL)/notifyv4

$(SUIT_KEY) $(SUIT_PUB):
	@$(RIOTBASE)/dist/tools/suit_v4/gen_key.py

$(SUIT_PUB_HDR): $(SUIT_PUB)
	@xxd -i $(SUIT_PUB) > $@

suit/genkey: $(SUIT_KEY) $(SUIT_PUB) $(SUIT_PUB_HDR)