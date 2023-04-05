## svcmgr
A very simple Windows app that puts itself in the tray.

Add a shortcut to `shell:startup` and enjoy

Tray icon:
- uses _r/stopped.ico when service is stopped
- uses _r/running.ico when service is started

Clicking the tray icon opens a dialog where you can see/set config values.

A few easy to replace values in the code:
- `MY_SERVICE_NAME` the actual service name (minus the .exe; what shows in services.msc)
- `MY_SERVICE_NAME_FRIENDLY` the friendly name (shows as tooltip on hover)

The config for this program is looking in the registry:
- Looks under "HKEY_LOCAL_MACHINE\Software\CompanyName" (`MY_SERVICE_REGISTRY_PATH`)
- Looks for path to folder under "MyServiceInstallPath" (`MY_SERVICE_REGISTRY_INSTALL_PATH_KEY`)
- Tries to open a config "config.bin" (`CONFIG_FILE_NAME`)

You can replace the values in config.cpp (`MY_SERVICE_REGISTRY_PATH`, etc) or remove that altogether.
