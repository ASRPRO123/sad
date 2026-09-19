# manifest.py - Frozen Python modules + C module declarations
#
# This file is placed into the board directory at build time.
# It includes the port's default manifest, then registers both C modules.

include("$(PORT_DIR)/boards/manifest.py")

# translate C module (copied into ports/esp32/modules/translate by CI)
c_module("$(PORT_DIR)/modules/translate")

# ulab C module (copied from the ulab repo into ports/esp32/modules/ulab by CI)
c_module("$(PORT_DIR)/modules/ulab")
