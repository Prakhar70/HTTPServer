// service.cpp

#include "service.hpp"
//TODO-Comments

SERVICE_STATUS g_ServiceStatus = {};

// Service status structure updated by SCM calls
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;

// Global pointers to server and async handler used across control flow
LLMServer* g_ServerInstance = nullptr;
std::thread g_ServerThread;
TAsyncHndlr* g_AsyncHndlr = nullptr;


void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
////TODO-Comments


void RunAsService() {
    // Define the service table for SCM: name + entry function
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { const_cast<LPTSTR>(TEXT("OfflineAIService")), ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcher(serviceTable)) {
        RunAsConsoleFallback();
    }
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {

    g_StatusHandle = RegisterServiceCtrlHandler(TEXT("OfflineAIService"), ServiceCtrlHandler);
    if (!g_StatusHandle) return;

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    // Specify that this service runs as a standalone process using the standard Windows (Win32) API.
    //Even when you're on x64, the Windows API is still officially the "Win32 API"
    
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    //Allows the service to accept stop requests from the Service Control Manager.
    

    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    //It’s a polite and expected handshake with Windows — "starting now..." → "all set!"

    g_ServerInstance = new LLMServer(3000);
    g_AsyncHndlr = new TAsyncHndlr();
    g_AsyncHndlr->Initialize(new LLMReqProcessor(), 15);

    g_ServerInstance->SetKeepAlive(true);

    if (!g_ServerInstance->Initialize()) {
        printf("[Service] Initialization failed.\n");
        return;
    }

    g_ServerThread = std::thread([]() {
        g_ServerInstance->RunMainLoop();
        });

    WaitForSingleObject(g_ServerInstance->GetStopEvent(), INFINITE);

    if (g_ServerThread.joinable()) {
        g_ServerThread.join();
    }

    delete g_ServerInstance;
    g_ServerInstance = nullptr;
    delete g_AsyncHndlr;
    g_AsyncHndlr = nullptr;

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
        case SERVICE_CONTROL_STOP:
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

            if (g_ServerInstance) {
                SetEvent(g_ServerInstance->GetStopEvent());
            }
            break;

        default:
            break;
    }
}


void RunAsConsoleFallback() {
    printf("[INFO] Running as console app (fallback)\n");

    SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL {
        if (ctrlType == CTRL_C_EVENT || ctrlType == CTRL_CLOSE_EVENT ||
            ctrlType == CTRL_BREAK_EVENT || ctrlType == CTRL_LOGOFF_EVENT || ctrlType == CTRL_SHUTDOWN_EVENT) {
            printf("[Console] Shutdown signal received (Ctrl+C or window close).\n");

            if (g_ServerInstance) {
                SetEvent(g_ServerInstance->GetStopEvent());
            }

            return TRUE;
        }
        return FALSE;
        }, TRUE);

    LLMServer server(3000);
    TAsyncHndlr asyn_hdlr;
    g_AsyncHndlr = &asyn_hdlr;
    
    g_AsyncHndlr->Initialize(new LLMReqProcessor(), 15);

    g_ServerInstance = &server;

    g_ServerInstance->SetKeepAlive(true);

    if (!server.Initialize()) {
        printf("[Console] Initialization failed.\n");
        return;
    }

    std::thread serverThread([]() {
        g_ServerInstance->RunMainLoop();
        });

    WaitForSingleObject(server.GetStopEvent(), INFINITE);

    if (serverThread.joinable()) {
        serverThread.join();
    }

    g_ServerInstance = nullptr;
    g_AsyncHndlr = nullptr;
    printf("[Console] Shutdown complete. Exiting.\n");
    exit(0);
}