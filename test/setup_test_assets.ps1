﻿# 手工用例测试资产准备脚本（对应 spec/main_test_cases.md 通用前置条件 P3）
# 用法：在 PowerShell 中执行  .\setup_test_assets.ps1
# 创建 %TEMP%\floaty_test\ 及其中全部测试资产；重复执行会先清空重建，保证可复用。

$ErrorActionPreference = "Stop"

$root = Join-Path $env:TEMP "floaty_test"
if (Test-Path $root) {
    Remove-Item $root -Recurse -Force
}
New-Item -ItemType Directory -Path $root | Out-Null
New-Item -ItemType Directory -Path (Join-Path $root "subfolder") | Out-Null

# 脚本资产：执行后在 %TEMP% 生成标记文件
$scriptBody = "@echo off`r`necho executed> `"%TEMP%\floaty_marker.txt`"`r`n"
Set-Content -Path (Join-Path $root "hello.bat") -Value $scriptBody -Encoding ASCII
Set-Content -Path (Join-Path $root "hello.cmd") -Value $scriptBody -Encoding ASCII

# 文档资产：文本与最小有效 PNG（1x1 像素）
Set-Content -Path (Join-Path $root "note.txt") -Value "floaty test note" -Encoding ASCII
$pngBase64 = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg=="
[IO.File]::WriteAllBytes((Join-Path $root "pic.png"), [Convert]::FromBase64String($pngBase64))

# 快捷方式资产：指向画图程序（TC-F3-02 / TC-F4-08 用）
$shell = New-Object -ComObject WScript.Shell
$lnk = $shell.CreateShortcut((Join-Path $root "test.lnk"))
$lnk.TargetPath = Join-Path $env:SystemRoot "System32\mspaint.exe"
$lnk.Save()

# 白名单外类型资产（TC-F4-05 / F16 拖放拒绝用）
Set-Content -Path (Join-Path $root "test.reg") -Value "Windows Registry Editor Version 5.00" -Encoding ASCII
Set-Content -Path (Join-Path $root "test.ps1") -Value "Write-Host test" -Encoding ASCII

# F12 长名称资产：名称宽度超过面板最大宽度，用于自适应与跑马灯人工/自动核对
$longName = ("超长快捷项名称" * 6) + ".txt"
Set-Content -Path (Join-Path $root $longName) -Value "long name item" -Encoding ASCII

# F16 拖放资产：三个合法文本与一个白名单外文件
Set-Content -Path (Join-Path $root "drop1.txt") -Value "drop one" -Encoding ASCII
Set-Content -Path (Join-Path $root "drop2.txt") -Value "drop two" -Encoding ASCII
Set-Content -Path (Join-Path $root "drop3.txt") -Value "drop three" -Encoding ASCII
Set-Content -Path (Join-Path $root "drop_bad.reg") -Value "Windows Registry Editor Version 5.00" -Encoding ASCII

# 清理上次运行残留的脚本标记
$marker = Join-Path $env:TEMP "floaty_marker.txt"
if (Test-Path $marker) {
    Remove-Item $marker -Force
}

Write-Host "测试资产已创建：$root"
