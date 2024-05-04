#!/bin/bash
source /opt/esp-idf/export.sh
idf.py set-target esp32
idf.py build
idf.py flash monitor
exit 0
