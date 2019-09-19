#!/bin/bash
URL=$1
VRB_EXT=$2
VRB_APP=org.mozilla.vrbrowser
WAVEVR_EXT=wavevr
DEV_EXT=dev

if [ -z "$VRB_EXT" -o "$VRB_EXT" = "$WAVEVR_EXT" ] ; then
  VRBINSTALLED=`adb shell pm list packages $VRB_APP.$WAVEVR_EXT`
  if [ ! -z "$VRBINSTALLED" ] ; then
    VRB_EXT=.$WAVEVR_EXT
  elif [ -n "$VRB_EXT" ] ; then
    echo Error: $VRB_APP.$WAVEVR_EXT not installed.
    exit 1
  fi
fi

if [ -z "$VRB_EXT" -o "$VRB_EXT" = "$DEV_EXT" ] ; then
  VRBINSTALLED=`adb shell pm list packages $VRB_APP.$DEV_EXT`
  if [ ! -z "$VRBINSTALLED" ] ; then
    VRB_EXT=.$DEV_EXT
  elif [ -n "$VRB_EXT" ] ; then
    echo Error: $VRB_APP.$DEV_EXT not installed.
    exit 1
  fi
fi

if [ "$VRB_EXT" == "default" ] ; then
  VRB_EXT=""
fi
echo Using $VRB_APP$VRB_EXT

if [ "$URL" = x ] ; then
  URL=""
fi

if [ -z "$URL" ] ; then
exec adb shell am start -a android.intent.action.LAUNCH $VRB_APP$VRB_EXT/$VRB_APP.VRBrowserActivity
else
exec adb shell am start -a android.intent.action.VIEW -d "$URL" $VRB_APP$VRB_EXT/$VRB_APP.VRBrowserActivity
fi
