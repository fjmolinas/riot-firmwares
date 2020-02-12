# To use the same prefix for the demo subnet
export LOCAL_IP6_PREFIX="2001:6d6f:6c69:6e61"
# This should match the URL of the ota-server, so `--http-port`
# and `--http-host`
export SUIT_OTA_SERVER_URL="http://127.0.0.1:8888"
# Overrite suit/notify and suit/publish targets to use ota-server
# path will be of something like:
# home/user/riot-firmwares/Makefiles/suit.v4.http.mk
export RIOT_MAKEFILES_GLOBAL_POST=${SUIT_MAKEFILE_HTTP}
# If wants to override the application 
export APPLICATION=air_monitor
# Address for the SUIT_CLIENT, if the same prefix is used for the
# subnet then only the EUI64 bytes will change depending on the node
export SUIT_CLIENT="[${LOCAL_IP6_PREFIX}:7b7e:3255:1313:8d96]"
# Set nodes posicion only for single demos
export NODE_LAT="48.837832"
export NODE_LNG="2.102920"
