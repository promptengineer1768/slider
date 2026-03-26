; Inno Setup Script for Sliding Puzzle
; This script creates a Windows installer for the Slider game

#define AppName "Sliding Puzzle"
#define AppVersion "1.0.1"
#define AppPublisher "Sliding Puzzle Team"
#define AppURL "https://github.com/promptengineer1768/slider"
#define AppExeName "slider.exe"
#define BuildDir "..\build\windows-msvc-release"

[Setup]
AppId={{e8d2e8b6-94e2-45a7-93e1-32c589b9f911}}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=..\dist\windows
OutputBaseFilename=slider-{#AppVersion}-win64
SetupIconFile=..\resources\icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UsePreviousTasks=no
UninstallDisplayIcon={app}\bin\{#AppExeName}
UninstallDisplayName={#AppName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce

[Files]
Source: "{#BuildDir}\bin\{#AppExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "{#BuildDir}\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist
; Only ship runtime assets from bin/resources (sounds + theme config). Icon sources are handled separately.
Source: "{#BuildDir}\bin\resources\*.wav"; DestDir: "{app}\bin\resources"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\bin\resources\themes.json"; DestDir: "{app}\bin\resources"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\resources\icon.ico"; DestDir: "{app}\bin"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; IconFilename: "{app}\bin\icon.ico"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\{#AppName}"; Filename: "{app}\bin\{#AppExeName}"; IconFilename: "{app}\bin\icon.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
