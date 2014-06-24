
#set -x

source config.zsh

excluded_modules="tools/touchprobe tools/zprobe tools/laser utils/panel"
#excluded_modules="$excluded_modules utils/player"
#excluded_modules="$excluded_modules tools/temperaturecontrol"
#excluded_modules="$excluded_modules tools/switch"

make_options=()

#export BUILD_TYPE=Debug
export NONETWORK=1

if [[ -n $excluded_modules ]]; then
  echo "-- exclude modules: $excluded_modules"
  #export EXCLUDED_MODULES=$excluded_modules
  make_options=(EXCLUDED_MODULES="$excluded_modules")
fi


if make $make_options all; then
  if $upload_after_build; then
    source upload.zsh
  fi
  if $burn_after_build; then
    source burn.zsh
  fi
fi
