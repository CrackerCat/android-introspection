if (ANDROID)
  set(MY_TARGET "android-${ANDROID_ABI}")
elseif (WASM)
  set(MY_TARGET "wasm")
elseif (HOST)
  set(MY_TARGET "host")
else ()
  set(MY_TARGET "generic")
endif ()