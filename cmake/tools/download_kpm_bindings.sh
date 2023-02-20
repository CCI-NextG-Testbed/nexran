#!/bin/bash

set -x

GENERATED_FULL_DIR=$1
shift
done_flag="$GENERATED_FULL_DIR"/done
exists=0
if [ -f "$done_flag" ]; then
    exists=1
fi
if [ $exists -eq 0 ]; then
   mkdir -p "$GENERATED_FULL_DIR"
   curl https://www.emulab.net/downloads/johnsond/profile-oai-oran/E2SM-KPM-ext-generated-bindings.tar.gz | tar -xzv -C "${GENERATED_FULL_DIR}" --strip-components 1
fi
touch $done_flag
