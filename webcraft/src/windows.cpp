#ifdef _WIN32

#include <windows.h>
#include "webcraft/webcraft.hpp"

/*
https://github.com/lewissbaker/cppcoro/blob/master/lib/io_service.cpp#L308
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/io_service.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/readable_file.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/file_read_operation.hpp
https://github.com/lewissbaker/cppcoro/blob/master/lib/file_read_operation.cpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/detail/win32_overlapped_operation.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/detail/win32.hpp
https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/file.hpp
https://github.com/lewissbaker/cppcoro/tree/master?tab=readme-ov-file#io_service-and-io_work_scope
https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-overlapped

essentially to run it with windows what you need to do is:

create an IoCompletionPort (this will act as your queue of events)

then create your io device and some threads if you want

then create a thread pool and assign the threads to the IoCompletionPort

then create your async task and assign it to the IoCompletionPort
 - Just call teh ReadFile, WriteFile, etc. functions with the IoCompletionPort as the first argument

then wait for the events to be processed in the thread pool
 - Just call the GetQueuedCompletionStatus function with the IoCompletionPort as the first argument

pseudocode:

initWinSock()
handle = CreateIoCompletionPort()
task = RunTask() // read from file and print from console
coroutine = task.coro

coroutine.resume() // start the task wait for it to make the "first" async call (ReadFile, WriteFile, etc.) then have it suspend

Overlapped* ov;
while GetQueuedCompletionStatus(handle, ov, ...) {

    callback = ((Event) ov)->callback
    callback(); // will resume coroutine here
}



*/

void run_app()
{
    fmt::println("Running on Windows");
}

#endif