#pragma once

// 单实例（非功能需求）：以命名互斥体保证同一时刻只有一个 Floaty 进程。
// 返回 true 表示本进程成为唯一实例；返回 false 表示已有实例在运行，调用方应直接退出。
bool acquireSingleInstance();
