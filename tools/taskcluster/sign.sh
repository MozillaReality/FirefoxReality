#!/bin/sh
python tools/taskcluster/fetch_secret.py -s project/firefoxreality/autograph_token -o token -n password
python tools/taskcluster/sign_apk.py -t token
python tools/taskcluster/archive_debug_apk.py
