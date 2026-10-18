; 浮启 Floaty 安装包脚本（Inno Setup 6）
; 构建：ISCC.exe installer\floaty.iss，产物输出到 dist\FloatySetup-2.3.0.exe
; 注意：本文件需以 UTF-8 with BOM 保存，否则中文显示乱码

#define MyAppName "浮启 Floaty"
#define MyAppVersion "2.3.0"
#define MyAppPublisher "Floaty"
#define MyAppExeName "Floaty.exe"

[Setup]
; 卸载注册表项用的固定 AppId，勿随版本变更
AppId={{3F8E7A1C-9B2D-4E5F-A6C8-1D3B5E7F9A02}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
; 免管理员安装：装进用户目录，未签名的开源小工具避免触发 UAC
DefaultDirName={localappdata}\Programs\Floaty
DefaultGroupName={#MyAppName}
PrivilegesRequired=lowest
OutputDir=..\dist
OutputBaseFilename=FloatySetup-{#MyAppVersion}
SetupIconFile=..\resources\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; 安装时若正在运行则尝试关闭（单实例，直接退出即可）
CloseApplications=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; 发布内容 = windeployqt 部署后的完整绿色目录，剔除开发用工具与测试程序
Source: "..\build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "make_icon.exe,FloatyTests.exe,*.pdb,*.ilk"
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
