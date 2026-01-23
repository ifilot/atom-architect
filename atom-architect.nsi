; =========================================
; Atom Architect NSIS Installer
; =========================================

!include "MUI2.nsh"
!include "x64.nsh"

; -----------------------------------------
; Application metadata
; -----------------------------------------
!define APP_EXE         "atom-architect.exe"
!define COMPANY_NAME    "Inorganic Materials & Catalysis"
!define INSTALL_DIR     "$PROGRAMFILES64\${APP_NAME}"

; -----------------------------------------
; Installer output
; -----------------------------------------
Name "${APP_NAME} ${VERSION}"
OutFile "${APP_NAME}-${VERSION}-setup.exe"
InstallDir "${INSTALL_DIR}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "InstallDir"

RequestExecutionLevel admin

; -----------------------------------------
; Installer icon
; -----------------------------------------
Icon "assets/icons/atom_architect_256.ico"

; -----------------------------------------
; UI pages
; -----------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; =========================================
; INSTALL SECTION
; =========================================
Section "Install"

  SetOutPath "$INSTDIR"

  ; Copy deployed application (exe + Qt DLLs + plugins)
  File /r "dist\*.*"

  ; Registry
  WriteRegStr HKLM "Software\${APP_NAME}" "InstallDir" "$INSTDIR"

  ; Uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Uninstall entry (Apps & Features)
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "DisplayName" "${APP_NAME} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "Publisher" "${COMPANY_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "DisplayVersion" "${VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
      "NoRepair" 1

  ; Start Menu shortcut (uses embedded EXE icon)
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" \
      "$INSTDIR\${APP_EXE}"

  ; Optional desktop shortcut
  ; CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

SectionEnd

; =========================================
; UNINSTALL SECTION
; =========================================
Section "Uninstall"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  RMDir "$SMPROGRAMS\${APP_NAME}"

  ; Remove files
  RMDir /r "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
  DeleteRegKey HKLM "Software\${APP_NAME}"

SectionEnd