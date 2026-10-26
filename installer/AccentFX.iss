; Inno Setup script -> AccentFX-Setup.exe
#define AppName "Accent FX"
#define AppVer  "1.0.0"
#define Art     "..\build\AccentFX_artefacts\Release"

[Setup]
AppId={{6C1F5B0E-7A32-4C0B-9E41-ACCE17F00001}
AppName={#AppName}
AppVersion={#AppVer}
AppPublisher=AccentLabs
DefaultDirName={autopf64}\{#AppName}
DefaultGroupName={#AppName}
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
OutputDir=..\dist
OutputBaseFilename=AccentFX-Setup
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
UninstallDisplayName={#AppName}
WizardStyle=modern

[Types]
Name: "full"; Description: "VST3 plugin + Standalone"
Name: "vst"; Description: "VST3 plugin only"
Name: "custom"; Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plugin"; Types: full vst custom; Flags: fixed
Name: "standalone"; Description: "Standalone application"; Types: full custom

[Files]
; a .vst3 on Windows is a folder bundle
Source: "{#Art}\VST3\Accent FX.vst3\*"; DestDir: "{commoncf64}\VST3\Accent FX.vst3"; Flags: recursesubdirs ignoreversion; Components: vst3
Source: "{#Art}\Standalone\Accent FX.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: standalone

[Icons]
Name: "{group}\Accent FX"; Filename: "{app}\Accent FX.exe"; Components: standalone
Name: "{group}\Uninstall Accent FX"; Filename: "{uninstallexe}"
