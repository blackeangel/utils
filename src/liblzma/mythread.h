/* Minimal stub: threading disabled (XZ_ENABLE_THREADS=OFF) */
#pragma once
#include <stdbool.h>
#define mythread_once(func) do { static bool _done = false; if(!_done){_done=true;func();} } while(0)
#define MYTHREAD_ONCE_INIT false
typedef bool mythread_once_t;
