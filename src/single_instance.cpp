#include "single_instance.h"

#include <windows.h>

bool acquireSingleInstance()
{
    // 会话级命名互斥体：同一用户会话内只允许一个 Floaty 实例。
    // 句柄有意不关闭，由进程退出时由系统回收；进程存续期间互斥体始终被持有。
    HANDLE handle = ::CreateMutexW(nullptr, FALSE, L"Local\\Floaty.SingleInstance");
    if (handle == nullptr) {
        return false;
    }
    return ::GetLastError() != ERROR_ALREADY_EXISTS;
}
