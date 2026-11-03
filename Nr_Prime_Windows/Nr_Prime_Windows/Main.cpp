#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int childMain() {
    int a, b;
    cin >> a >> b;
    for (int n = a; n <= b; ++n)
        if (isPrime(n)) cout << n << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && string(argv[1]) == "child")
        return childMain();

    const int NUM_PROCS = 10;
    const int RANGE = 1000;

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    HANDLE readHandles[NUM_PROCS];
    HANDLE procHandles[NUM_PROCS];

    for (int i = 0; i < NUM_PROCS; ++i) {
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

        HANDLE inRead, inWrite;
        HANDLE outRead, outWrite;
        CreatePipe(&inRead, &inWrite, &sa, 0);
        CreatePipe(&outRead, &outWrite, &sa, 0);

        SetHandleInformation(inWrite, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = inRead;
        si.hStdOutput = outWrite;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        PROCESS_INFORMATION pi = {};
        string cmd = "\"" + string(exePath) + "\" child";
        if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            cerr << "CreateProcess a esuat: " << GetLastError() << "\n";
            return 1;
        }

        CloseHandle(inRead);
        CloseHandle(outWrite);

        string msg = to_string(i * RANGE) + " " + to_string((i + 1) * RANGE - 1) + "\n";
        DWORD written;
        WriteFile(inWrite, msg.c_str(), (DWORD)msg.size(), &written, NULL);
        CloseHandle(inWrite);

        readHandles[i] = outRead;
        procHandles[i] = pi.hProcess;
        CloseHandle(pi.hThread);
    }

    for (int i = 0; i < NUM_PROCS; ++i) {
        string data;
        char buf[512];
        DWORD n;
        while (ReadFile(readHandles[i], buf, sizeof(buf), &n, NULL) && n > 0)
            data.append(buf, n);

        cout << "Procesul " << i << " (" << i * RANGE << " - " << (i + 1) * RANGE - 1 << "):\n";
        istringstream iss(data);
        int p;
        while (iss >> p) cout << p << " ";
        cout << "\n\n";

        CloseHandle(readHandles[i]);
        WaitForSingleObject(procHandles[i], INFINITE);
        CloseHandle(procHandles[i]);
    }
    return 0;
}